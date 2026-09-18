#ifndef AGRemapPyBind_PyRefFunction_H
#define AGRemapPyBind_PyRefFunction_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/typing.h>


namespace py = pybind11;


/**
 * @brief
 @rst
 The parameter / return type a binding declares for a callable it converts with
 :cpp:func:`toPyRefFunction`, so the generated stub still reads ``Callable[[...], R] | None`` rather
 than ``Any``. It accepts any object; the conversion does the checking
 @endrst
 *
 * @tparam Sig The signature to NAME in the stub -- a registered `Python`_ type in place of any core type
 *    that has none (eg. ``PyIniGroupedResource&`` for ``AGRemapCore::IniGroupedResource&``)
 */
template <typename Sig>
using PyOptionalCallable = py::typing::Optional<py::typing::Callable<Sig>>;


template <typename Sig>
struct PyRefFunction;

namespace PyRefFunctionDetail {
    template <typename Sig>
    std::function<Sig> toFunction(const py::object& fn);

    // A non-const lvalue reference is handed over BY REFERENCE, so an edit the callable makes lands
    // on the caller's object. Everything else keeps pybind11's usual conversion.
    template <typename A>
    py::object toPyArg(std::remove_reference_t<A>& arg) {
        if (std::is_lvalue_reference_v<A> && !std::is_const_v<std::remove_reference_t<A>>) {
            return py::cast(&arg, py::return_value_policy::reference);
        }

        return py::cast(arg);
    }

    template <typename R>
    struct FromPyResult {
        static R convert(py::object result) {
            return result.template cast<R>();
        }
    };

    template <>
    struct FromPyResult<void> {
        static void convert(py::object) {}
    };

    // A callable that RETURNS a callable (eg. a light map edit built per diffuse path) gets the same
    // treatment one level down -- through pybind11's own caster the returned callable would copy its
    // reference arguments all over again.
    template <typename R, typename... Args>
    struct FromPyResult<std::function<R(Args...)>> {
        static std::function<R(Args...)> convert(py::object result) {
            return toFunction<R(Args...)>(result);
        }
    };
}


/**
 * @brief
 @rst
 A `Python`_ callable held as a ``std::function<R(Args...)>`` that passes every non-const lvalue
 reference argument BY REFERENCE :raw-html:`<br />` :raw-html:`<br />`

 Letting ``pybind11/functional.h`` convert the callable instead compiles, runs, and loses every
 edit: its wrapper casts each argument with ``return_value_policy::automatic_reference``, which for
 an lvalue reference means a COPY. A copyable argument (a ``TextureFile``, a ``CachedFileStats``) is
 edited on a temporary the caller never sees; a non-copyable one throws ``cast_error`` inside the C++
 caller. Neither shows when the same object was handed to `Python`_ first, because the cast then
 finds the existing wrapper -- which is how every such binding passed its tests :raw-html:`<br />`
 :raw-html:`<br />`

 The callable is held through a ``shared_ptr`` whose deleter takes the GIL, because the function is
 copied around and destroyed by C++ code that does not hold it. A named type rather than a lambda so
 :cpp:func:`fromPyRefFunction` can hand the ORIGINAL `Python`_ object back through
 ``std::function::target``
 @endrst
 *
 * @tparam R The return type
 * @tparam Args The argument types
 */
template <typename R, typename... Args>
struct PyRefFunction<R(Args...)> {
    std::shared_ptr<py::object> callable;

    R operator()(Args... args) const {
        py::gil_scoped_acquire gil;
        py::object result = (*callable)(PyRefFunctionDetail::toPyArg<Args>(args)...);
        return PyRefFunctionDetail::FromPyResult<R>::convert(std::move(result));
    }
};


namespace PyRefFunctionDetail {
    template <typename Sig>
    std::function<Sig> toFunction(const py::object& fn) {
        if (fn.is_none()) {
            return {};
        }

        if (!PyCallable_Check(fn.ptr())) {
            throw py::type_error("Expected a callable or None");
        }

        std::shared_ptr<py::object> held(new py::object(fn), [](py::object* obj) {
            // Past interpreter shutdown there is no GIL to take, and nothing left to release
            if (!Py_IsInitialized()) {
                return;
            }

            py::gil_scoped_acquire gil;
            delete obj;
        });

        return PyRefFunction<Sig>{std::move(held)};
    }
}


/**
 * @brief
 @rst
 Converts a `Python`_ callable -- or ``None``, for an empty function -- into a ``std::function``
 whose non-const reference arguments reach `Python`_ by reference; see :cpp:class:`PyRefFunction`
 :raw-html:`<br />` :raw-html:`<br />`

 Take the argument as a ``py::object`` and convert it with this whenever the signature has a
 non-const reference anywhere in it, including in the signature of a callable it RETURNS. Taking
 the ``std::function`` directly (or binding it with ``def_readwrite``) is the bug
 @endrst
 *
 * @tparam Sig The function signature, eg. ``bool(AGRemapCore::IniGroupedResource&)``
 * @param fn The callable, or ``None``
 * @return The function, empty for ``None``
 * @throws py::type_error If 'fn' is neither callable nor ``None``
 */
template <typename Sig>
std::function<Sig> toPyRefFunction(const py::object& fn) {
    return PyRefFunctionDetail::toFunction<Sig>(fn);
}


/**
 * @brief
 @rst
 The getter half of :cpp:func:`toPyRefFunction`: the ORIGINAL `Python`_ callable for a function it
 built, ``None`` for an empty one, and `pybind11`_'s own wrapper for a function that came from C++
 @endrst
 *
 * @tparam Sig The function signature
 * @tparam HintSig The signature the stub names -- see :cpp:type:`PyOptionalCallable`
 * @param fn The function
 * @return The `Python`_ object for 'fn'
 */
template <typename Sig, typename HintSig = Sig>
PyOptionalCallable<HintSig> fromPyRefFunction(const std::function<Sig>& fn) {
    py::object result;
    if (!fn) {
        result = py::none();
    } else if (const PyRefFunction<Sig>* held = fn.template target<PyRefFunction<Sig>>()) {
        result = *held->callable;
    } else {
        result = py::cast(fn);
    }

    return py::reinterpret_steal<PyOptionalCallable<HintSig>>(result.release());
}

#endif
