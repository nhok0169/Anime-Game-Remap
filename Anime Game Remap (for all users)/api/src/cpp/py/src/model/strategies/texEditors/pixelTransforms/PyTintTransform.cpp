#include "PyTintTransform.h"

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/TintTransform.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTintTransform(pybind11::module_ &m) {
    py::class_<AGRC::TintTransform, AGRC::BasePixelTransform, py::smart_holder>(m, "CppTintTransform", R"doc(
This class inherits from :class:`CppBasePixelTransform`

Controls the tint of a texture file using the `Simple Image Temperature/Tint Adjust Algorithm`_
    )doc")

        .def(py::init<int>(), py::arg("tint") = 0, py::doc(R"doc(
Constructs a new tint pixel transform

Parameters
----------
tint: :class:`int`
    The tint to set the image. Range from -100 to 100. **Default**: ``0``
        )doc"))

        .def_readwrite("tint", &AGRC::TintTransform::tint, py::doc(R"doc(
:class:`int`: The tint to set the image. Range from -100 to 100
        )doc"));
}
