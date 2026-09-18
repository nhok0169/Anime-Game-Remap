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

#include "PyHueAdjust.h"

#include "AGRemapCore/model/strategies/texEditors/texFilters/HueAdjust.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "PyTexFilterCommon.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppHueAdjust(pybind11::module_ &m) {
    py::class_<AGRC::HueAdjust, AGRC::BaseTexFilter, py::smart_holder>(m, "CppHueAdjust", R"doc(
This class inherits from :class:`CppBaseTexFilter`

Adjusts the hue of a texture file
    )doc")

        .def(py::init<int>(), py::arg("hue"), py::doc(R"doc(
Constructs a new hue-adjust filter

Parameters
----------
hue: :class:`int`
    The hue to adjust the image. Value is from -180 to 180
        )doc"))

        .def_readwrite("hue", &AGRC::HueAdjust::hue, py::doc(R"doc(
:class:`int`: The hue to adjust the image. Value is from -180 to 180
        )doc"))

        .def("transform", [](AGRC::HueAdjust &self, py::object texFileObj) {
            AGRC::TextureFile &texFile = syncTextureFileFromImg(texFileObj);
            self.transform(texFile);
            syncTextureFileToImg(texFileObj);
        }, py::arg("texFile"), py::doc(R"doc(
Adjusts the hue across the image

Parameters
----------
texFile: :class:`TextureFile`
    The texture to be edited
        )doc"));
}
