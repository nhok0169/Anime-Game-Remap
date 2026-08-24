#include "PyInvertAlpha.h"

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/InvertAlpha.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppInvertAlpha(pybind11::module_ &m) {
    py::class_<AGRC::InvertAlpha, AGRC::BasePixelTransform, py::smart_holder>(m, "CppInvertAlpha", R"doc(
This class inherits from :class:`CppBasePixelTransform`

Inverts the alpha channel of a pixel
    )doc")

        .def(py::init<>());
}
