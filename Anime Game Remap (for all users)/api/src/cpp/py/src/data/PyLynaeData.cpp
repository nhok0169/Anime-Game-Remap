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
#include <pybind11/stl.h>

#include "AGRemapCore/data/IniFixData/Lynae/LynaeBangsShader.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeFilmShader.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeHairShader.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeLowerShader.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeTextures.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeUpperShader.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {
    // the data's C++ functions, as a class only holds them: nothing is constructed
    struct LynaeData {};
}


void initCppLynaeData(pybind11::module_ &m) {
    py::class_<LynaeData>(m, "LynaeData", R"doc(
Lynae's data from WuWa, as her fix and parser are built from it: the game shaders it carries onto her skin, her
props film flags and her texture facts. Each is the data a
:class:`WWMIFixerConfig` or :class:`WWMIParserConfig` for Lynae is given; nothing here is built
    )doc")
        .def_static("bangsShader", &AGRC::lynaeBangsShader, ":class:`WWMIFixerConfig.Shader`: Lynae's bangs pass")
        .def_static("hairShader", &AGRC::lynaeHairShader, ":class:`WWMIFixerConfig.Shader`: Lynae's hair pass")
        .def_static("filmShader", &AGRC::lynaeFilmShader, ":class:`WWMIFixerConfig.Shader`: Lynae's coat film pass")
        .def_static("upperShader", &AGRC::lynaeUpperShader, ":class:`WWMIFixerConfig.Shader`: Lynae's upper-body pass")
        .def_static("lowerShader", &AGRC::lynaeLowerShader, ":class:`WWMIFixerConfig.Shader`: Lynae's lower-body pass")
        .def_static("propsFilmFlags", &AGRC::lynaePropsFilmFlags,
            ":class:`WWMIParserConfig.DrawSplit.FlagMask`: Where Lynae's own props code map carries her film flag")
        .def_static("textureFacts", &AGRC::lynaeTextureFacts, ":class:`WWMITextureFacts`: Lynae's texture roles and registers (her thumbprints are her :class:`TextureThumbprints` rows)");
}
