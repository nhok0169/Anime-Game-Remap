#include "PyZ3Context.h"

#include "AGRemapCore/tools/z3/Z3Context.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppZ3Context(pybind11::module_ &m) {
    // Move-only in C++ (Z3Context(const Z3Context&) = delete) -- pybind11's default holder
    // (std::unique_ptr<T>) handles that fine, and py::init<>() constructs directly into the
    // holder (no separate move of a T value ever needed), so nothing extra is required here.
    py::class_<AGRC::Z3Context>(m, "Z3Context", R"doc(
An opaque handle to a `Z3`_ context

Every named variable used across several :class:`IfPredPart`/:class:`Z3Predicate` values that
share the same :class:`Z3Context` refers to the same underlying `Z3`_ constant -- construct one
:class:`Z3Context` per logical group of predicates that should be comparable/combinable together
(eg. one per .ini file being read), not a fresh one per predicate.
    )doc")
        .def(py::init<>());
}
