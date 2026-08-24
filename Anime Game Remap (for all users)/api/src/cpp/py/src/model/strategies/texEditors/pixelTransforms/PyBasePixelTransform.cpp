#include "PyBasePixelTransform.h"

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBasePixelTransform(pybind11::module_ &m) {
    py::class_<AGRC::BasePixelTransform, py::smart_holder>(m, "CppBasePixelTransform", R"doc(
Base class for transforming a pixel in a texture file

.. container:: operations

    **Supported Operations:**

    .. describe:: x(pixel, xCoord, yCoord)

        Calls :meth:`transform` for the pixel transform, ``x``
    )doc")

        .def(py::init<>())

        // See PyBaseTexFilter.cpp's identical '__call__' binding for why this goes through
        // Python's own attribute lookup instead of AGRC::BasePixelTransform::operator() directly.
        .def("__call__", [](py::object self, AGRC::Colour &pixel, int x, int y) {
            self.attr("transform")(pixel, x, y);
        }, py::arg("pixel"), py::arg("x"), py::arg("y"), py::doc(R"doc(
Calls :meth:`transform` for the pixel transform
        )doc"))

        .def("transform", &AGRC::BasePixelTransform::transform, py::arg("pixel"), py::arg("x"), py::arg("y"), py::doc(R"doc(
Applies a transformation to 'pixel'. No-op by default

Parameters
----------
pixel: :class:`CppColour`
    The pixel to be edited

x: :class:`int`
    x-coordinate of the pixel

y: :class:`int`
    y-coordinate of the pixel
        )doc"));
}
