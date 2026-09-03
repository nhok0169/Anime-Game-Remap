#ifndef AGRemapPyBind_PyStrategyFactory_H
#define AGRemapPyBind_PyStrategyFactory_H

#include <memory>
#include <utility>

#include <pybind11/pybind11.h>

namespace py = pybind11;


/**
 * @brief
 @rst
 Owns one `Python`_ object for exactly as long as a ``shared_ptr`` to its C++ half is alive
 :raw-html:`<br />` :raw-html:`<br />`

 The destructor re-acquires the GIL because the last ``shared_ptr`` may be released from C++, with
 no GIL held -- dropping a ``py::object`` there is the classic way to crash an interpreter
 @endrst
 */
struct PyStrategyKeepAlive {
    py::object obj;

    explicit PyStrategyKeepAlive(py::object object): obj(std::move(object)) {}

    ~PyStrategyKeepAlive() {
        py::gil_scoped_acquire gil;
        obj = py::object();
    }
};


/**
 * @brief
 @rst
 Turns what a `Python`_ strategy factory returned into the ``shared_ptr`` the core builders hand
 around, **without losing the object's `Python`_ identity** :raw-html:`<br />` :raw-html:`<br />`

 The subtlety this exists for: casting the result straight to a ``shared_ptr`` yields one that
 owns the *C++* half only. The moment the caller's last reference to the `Python`_ object goes, the
 ``PyObject`` is freed while the C++ object lives on -- and casting that back across the boundary
 later builds a **brand new** wrapper of the registered base type. A subclass written in `Python`_
 goes in and a plain base object comes out, quietly, with every attribute it carried gone
 :raw-html:`<br />` :raw-html:`<br />`

 So the returned ``shared_ptr`` is built with the *aliasing* constructor: it points at the C++
 object but its control block owns a #PyStrategyKeepAlive, which pins the `Python`_ object. Casting
 it back then finds the original instance in `pybind11`_'s registry and returns *that*
 @endrst
 *
 * @tparam PyStrategy The pybind11-facing class the factory is expected to return
 * @tparam CoreStrategy The core base the builder's Factory is declared in terms of
 *
 * @param result What the Python factory returned -- ``None`` yields ``nullptr``
 *
 * @throws pybind11::cast_error If 'result' is not a 'PyStrategy'
 *
 * @return The strategy, or ``nullptr``
 */
template <typename PyStrategy, typename CoreStrategy>
std::shared_ptr<CoreStrategy> holdPyStrategy(py::object result) {
    if (result.is_none()) {
        return nullptr;
    }

    PyStrategy* raw = result.template cast<PyStrategy*>();
    auto keepAlive = std::make_shared<PyStrategyKeepAlive>(std::move(result));

    return std::shared_ptr<CoreStrategy>(std::move(keepAlive), raw);
}

#endif
