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

#include "PyInvertAlphaFilter.h"

#include "AGRemapCore/model/strategies/texEditors/texFilters/InvertAlphaFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "PyTexFilterCommon.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppInvertAlphaFilter(pybind11::module_ &m) {
    py::class_<AGRC::InvertAlphaFilter, AGRC::BaseTexFilter, py::smart_holder>(m, "CppInvertAlphaFilter", R"doc(
This class inherits from :class:`CppBaseTexFilter`

Inverts the alpha channel of an image
    )doc")

        .def(py::init<>())

        .def("transform", [](AGRC::InvertAlphaFilter &self, py::object texFileObj) {
            AGRC::TextureFile &texFile = syncTextureFileFromImg(texFileObj);
            self.transform(texFile);
            syncTextureFileToImg(texFileObj);
        }, py::arg("texFile"), py::doc(R"doc(
Inverts the alpha channel across the image

Parameters
----------
texFile: :class:`TextureFile`
    The texture to be edited
        )doc"));
}
