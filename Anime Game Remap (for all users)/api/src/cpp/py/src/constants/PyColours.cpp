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

#include <pybind11/pybind11.h>

#include "AGRemapCore/constants/Colours.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppColours(pybind11::module_ &m) {
    py::class_<AGRC::Colours>(m, "Colours", R"doc(
Colours the texture edits share: a flat normal map or light map a fix creates or replaces a band with, and white

A colour that is ONE character's game data (the skin code in its material mask, say) is not here: it is a row of a
versioned asset table (:class:`MaterialColours`)
    )doc")
        .def_readonly_static("White", &AGRC::Colours::White, ":class:`CppColour`: Opaque white")
        .def_readonly_static("LightMapGreen", &AGRC::Colours::LightMapGreen, ":class:`CppColour`: The green a light map's flat (no-band) texels carry")
        .def_readonly_static("LightMapGreenMin", &AGRC::Colours::LightMapGreenMin,
                             ":class:`CppColour`: The low end of the light map's green band (see :attr:`ColourRanges.LightMapGreen`)")
        .def_readonly_static("LightMapGreenMax", &AGRC::Colours::LightMapGreenMax,
                             ":class:`CppColour`: The high end of the light map's green band (see :attr:`ColourRanges.LightMapGreen`)")
        .def_readonly_static("NormalMapYellow", &AGRC::Colours::NormalMapYellow, ":class:`CppColour`: A flat normal map in the yellow encoding (GI before 5.x)")
        .def_readonly_static("NormalMapBlue", &AGRC::Colours::NormalMapBlue, ":class:`CppColour`: A flat normal map in the blue (tangent space) encoding")
        .def_readonly_static("NormalMapPurple1", &AGRC::Colours::NormalMapPurple1,
                             ":class:`CppColour`: A flat normal map in the purple encoding, as GI 5.x writes its normal maps")
        .def_readonly_static("NormalMapPurple2", &AGRC::Colours::NormalMapPurple2,
                             ":class:`CppColour`: The second purple a GI 5.x normal map is flat at (a lighter green channel)")
        .def_readonly_static("NormalMapPurpleMin", &AGRC::Colours::NormalMapPurpleMin,
                             ":class:`CppColour`: The low end of the purple normal maps' bands (see :attr:`ColourRanges.NormalMapPurple1`)");

    py::class_<AGRC::ColourRanges>(m, "ColourRanges", R"doc(
The colour bands the texture edits share, each a :class:`CppColourRange` of :class:`Colours`
    )doc")
        .def_readonly_static("LightMapGreen", &AGRC::ColourRanges::LightMapGreen, ":class:`CppColourRange`: A light map's green band")
        .def_readonly_static("NormalMapPurple1", &AGRC::ColourRanges::NormalMapPurple1,
                             ":class:`CppColourRange`: A purple normal map's band, up to :attr:`Colours.NormalMapPurple1`")
        .def_readonly_static("NormalMapPurple2", &AGRC::ColourRanges::NormalMapPurple2,
                             ":class:`CppColourRange`: A purple normal map's band, up to :attr:`Colours.NormalMapPurple2`");
}
