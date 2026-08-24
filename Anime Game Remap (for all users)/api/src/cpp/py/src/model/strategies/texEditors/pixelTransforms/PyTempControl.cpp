#include "PyTempControl.h"

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/TempControl.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTempControl(pybind11::module_ &m) {
    py::class_<AGRC::TempControl, AGRC::BasePixelTransform, py::smart_holder>(m, "CppTempControl", R"doc(
This class inherits from :class:`CppBasePixelTransform`

Controls the temperature of a texture file using a modified version of the
`Simple Image Temperature/Tint Adjust Algorithm`_ such that the colour channels increase/decrease
linearly with respect to their corresponding pixel value and the user selected temperature
    )doc")

        .def(py::init<double>(), py::arg("temp") = 0, py::doc(R"doc(
Constructs a new temperature-control pixel transform

Parameters
----------
temp: :class:`float`
    The temperature to set the image. Range from -1 to 1. **Default**: ``0``
        )doc"))

        .def_readwrite("temp", &AGRC::TempControl::temp, py::doc(R"doc(
:class:`float`: The temperature to set the image. Range from -1 to 1
        )doc"));
}
