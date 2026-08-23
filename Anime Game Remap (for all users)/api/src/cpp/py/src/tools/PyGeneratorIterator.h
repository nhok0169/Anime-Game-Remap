#ifndef AGRemapPyBind_PyGeneratorIterator_H
#define AGRemapPyBind_PyGeneratorIterator_H

#include <functional>
#include <utility>

#include <pybind11/pybind11.h>

#include "AGRemapCore/tools/Generator.h"


namespace py = pybind11;


/**
 * @brief
 @rst
 Wraps `AGRC::Generator`\\<T\\> as a `Python`_-visible iterator (``__iter__``/``__next__``) --
 shared by every `pybind11`_ binding that exposes one of `IniSectionGraph`'s C++20-coroutine-based
 iterator methods (``iterSectsByContentPart``/``iterByContentPart``/``__iter__``/``iterByQuery``).
 One `pybind11`_ class registration is needed per distinct ``T``; register each with
 :cpp:func:`bindGeneratorIterator`, matching this shared C++ type template so the binding code
 itself isn't duplicated per method.
 @endrst
 *
 * @tparam T The type yielded by the wrapped generator
 */
template <typename T>
class PyGeneratorIterator {
    public:
        using Converter = std::function<py::object(T&)>;

        explicit PyGeneratorIterator(AGRemapCore::Generator<T> generator, Converter convert):
            generator_(std::move(generator)), convert_(std::move(convert)) {

        }

        /**
         * @brief Advances and converts the next value, or raises ``py::stop_iteration`` once exhausted
         */
        py::object next() {
            if (!generator_.next()) {
                throw py::stop_iteration();
            }
            return convert_(generator_.value());
        }

    private:
        AGRemapCore::Generator<T> generator_;
        Converter convert_;
};


/**
 * @brief Registers a `pybind11`_ class for `PyGeneratorIterator`\\<T\\>, under 'name'
 *
 * @tparam T The type yielded by the wrapped generator (must match some already-registered `Python`_ class)
 * @param m The module to register in
 * @param name The `Python`_-visible class name (eg. ``"SectionIterDataIterator"``)
 */
template <typename T>
void bindGeneratorIterator(pybind11::module_ &m, const char *name) {
    py::class_<PyGeneratorIterator<T>>(m, name)
        .def("__iter__", [](PyGeneratorIterator<T> &self) -> PyGeneratorIterator<T>& { return self; })
        .def("__next__", &PyGeneratorIterator<T>::next);
}

#endif
