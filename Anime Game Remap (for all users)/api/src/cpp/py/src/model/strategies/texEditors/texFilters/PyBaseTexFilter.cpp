#include "PyBaseTexFilter.h"

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "AGRemapCore/model/files/TextureFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBaseTexFilter(pybind11::module_ &m) {
    py::class_<AGRC::BaseTexFilter, py::smart_holder>(m, "CppBaseTexFilter", R"doc(
Base class for transforming a texture file

.. container:: operations

    **Supported Operations:**

    .. describe:: x(texFile)

        Calls :meth:`transform` for the filter, ``x``
    )doc")

        .def(py::init<>())

        // Deliberately NOT bound as a forward to AGRC::BaseTexFilter::operator() (which calls
        // this->transform(texFile) as a *C++* virtual call): a pure-Python subclass that overrides
        // only 'transform' (not '__call__') has no C++-side vtable entry for that override at all
        // without a trampoline, so a C++-internal virtual call would silently keep calling the
        // no-op base implementation instead. Routing through Python's own attribute lookup here
        // (self.attr("transform")) resolves to whatever 'transform' really is on the most-derived
        // object -- a C++-native override (eg. CppGammaFilter), or a pure-Python one -- exactly
        // like the pure-Python original's own '__call__' (self.transform(texFile)) did.
        .def("__call__", [](py::object self, AGRC::TextureFile &texFile) {
            self.attr("transform")(texFile);
        }, py::arg("texFile"), py::doc(R"doc(
Calls :meth:`transform` for the filter
        )doc"))

        .def("transform", &AGRC::BaseTexFilter::transform, py::arg("texFile"), py::doc(R"doc(
Applies a transformation to 'texFile'. No-op by default

Parameters
----------
texFile: :class:`CppTextureFile`
    The texture to be edited
        )doc"));
}
