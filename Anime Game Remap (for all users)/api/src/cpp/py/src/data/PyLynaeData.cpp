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
#include "AGRemapCore/data/IniFixData/Lynae/LynaeSkeleton.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeSkinCodes.h"
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
texture facts, her skeleton's numbering and shape-key order, and the tables of her texture edits. Each is the data a
:class:`WWMIFixerConfig` or :class:`WWMIParserConfig` for Lynae is given; nothing here is built
    )doc")
        .def_static("bangsShader", &AGRC::lynaeBangsShader, ":class:`WWMIFixerConfig.Shader`: Lynae's bangs pass")
        .def_static("hairShader", &AGRC::lynaeHairShader, ":class:`WWMIFixerConfig.Shader`: Lynae's hair pass")
        .def_static("filmShader", &AGRC::lynaeFilmShader, ":class:`WWMIFixerConfig.Shader`: Lynae's coat film pass")
        .def_static("upperShader", &AGRC::lynaeUpperShader, ":class:`WWMIFixerConfig.Shader`: Lynae's upper-body pass")
        .def_static("lowerShader", &AGRC::lynaeLowerShader, ":class:`WWMIFixerConfig.Shader`: Lynae's lower-body pass")
        .def_static("propsFilmFlags", &AGRC::lynaePropsFilmFlags,
            ":class:`WWMIParserConfig.DrawSplit.FlagMask`: Where Lynae's own props code map carries her film flag")
        .def_static("textureFacts", &AGRC::lynaeTextureFacts, ":class:`WWMITextureFacts`: Lynae's texture roles, registers and thumbprints")
        .def_static("renumbered36To37", &AGRC::lynaeRenumbered36To37,
            "Dict[:class:`int`, :class:`int`]: Lynae's bones renumbered from 3.6 to 3.7")
        .def_static("boneCentroids", &AGRC::lynaeBoneCentroids,
            "Dict[:class:`int`, Tuple[:class:`float`, :class:`float`, :class:`float`]]: Where each of Lynae's bones sits, to tell a mod's skeleton numbering")
        .def_static("vgMaps36", &AGRC::lynae36VgMaps,
            "Dict[:class:`int`, List[:class:`int`]]: Each of Lynae's components' vertex groups in her merged skeleton, at 3.6")
        .def_static("toPeppermintShapeKeys", &AGRC::lynaeToPeppermintShapeKeys,
            "List[:class:`int`]: Lynae's shape keys in LynaePeppermint's order")
        .def_static("skinCodeTable", &AGRC::lynaeSkinCodeTable,
            "List[:class:`int`]: The table that swaps the skin code between Lynae's and LynaePeppermint's body code maps, for :meth:`ChannelMapFilter.codeMap`")
        .def_static("filmCodeTable", &AGRC::lynaeFilmCodeTable,
            "List[:class:`int`]: The table that moves Lynae's coat film code to LynaePeppermint's, for :meth:`ChannelMapFilter.codeMap`")
        .def_static("propsPlainCodeTable", &AGRC::lynaePropsPlainCodeTable,
            "List[:class:`int`]: The table that moves Lynae's plain props materials to LynaePeppermint's plain hip-pouch material, for :meth:`ChannelMapFilter.codeMap`")
        .def_static("shadeBandFactors", &AGRC::lynaeShadeBandFactors,
            "List[Tuple[:class:`float`, :class:`float`, :class:`float`]]: Lynae's lower-body shading bands, as :class:`GuidedScaleFilter` factors guided by the normal map's alpha, in linear colour")
        .def_static("filmFactors", &AGRC::lynaeFilmFactors,
            "List[Tuple[:class:`float`, :class:`float`, :class:`float`]]: Lynae's coat film colour, as :class:`GuidedScaleFilter` factors guided by a jacket code map's codes");
}
