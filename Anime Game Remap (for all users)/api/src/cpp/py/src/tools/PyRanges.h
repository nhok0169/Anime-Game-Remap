#ifndef AGRemapPyBind_PyRanges_H
#define AGRemapPyBind_PyRanges_H

#include <optional>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/tools/Ranges.h"
 
 
namespace py = pybind11;
namespace AGRC = AGRemapCore; 

/**
 * @brief
 @rst
 The `pybind11`_ bound version of `Ranges`
 @endrst
 *
 * @tparam T The integer type used for range boundaries (matches `AGRemapCore::Ranges`'s template parameter)
 */
template <typename T>
class PyRanges: public AGRC::Ranges<T> {
    public:
        using AGRC::Ranges<T>::Ranges;
};
 
 
/**
 * @brief
 @rst
 The `pybind11`_ trampoline class for `PyRanges`, allowing `PyRanges` to be subclassed from `Python`_
 @endrst
 *
 * @tparam T The integer type used for range boundaries (matches `PyRanges`'s template parameter)
 */
template <typename T>
class PyBindRanges: public PyRanges<T> {
    public:
        using PyRanges<T>::PyRanges;
 
        bool has(T value) const override {
            PYBIND11_OVERRIDE(
                bool,
                PyRanges<T>,
                has,
                value
            );
        }
 
        bool isEmpty() const override {
            PYBIND11_OVERRIDE(
                bool,
                PyRanges<T>,
                isEmpty
            );
        }
 
        bool isFull() const override {
            PYBIND11_OVERRIDE(
                bool,
                PyRanges<T>,
                isFull
            );
        }
};
 
 
/**
 * @brief
 @rst
 Binds `PyRanges`\<T\> / `PyBindRanges`\<T\> to a `pybind11`_ module under the given Python class name
 @endrst
 *
 * @tparam T The integer type used for range boundaries
 *
 * @param m The module to bind the class into
 * @param className The name the class is exposed under in Python
 */
template <typename T>
void bindRangesClass(pybind11::module_ &m, const char *className);


/**
 * @brief
 @rst
 Converts a bound `PyRanges`\<T\> (the actual registered `pybind11`_ type) into the plain
 `AGRemapCore::Ranges`\<T\> that core APIs actually take -- an ordinary derived-to-base slice,
 not a deep adaptation: `PyRanges`\<T\> publicly inherits `AGRemapCore::Ranges`\<T\> and adds
 nothing but constructor inheritance
 @endrst
 *
 * @tparam T The integer type used for range boundaries
 *
 * @param ranges The bound Ranges instance to convert, if any
 *
 * @return The equivalent AGRemapCore::Ranges<T>, or ``std::nullopt`` if ``ranges`` was ``std::nullopt``
 */
template <typename T>
inline std::optional<AGRemapCore::Ranges<T>> toCoreRanges(const std::optional<PyRanges<T>> &ranges) {
    if (!ranges.has_value()) {
        return std::nullopt;
    }
    return AGRemapCore::Ranges<T>(ranges->ranges, false);
}
 
 
void initCppRanges(pybind11::module_ &m);

#endif
 