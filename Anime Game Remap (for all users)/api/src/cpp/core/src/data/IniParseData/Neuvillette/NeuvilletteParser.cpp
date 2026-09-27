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

#include "AGRemapCore/data/IniParseData/Neuvillette/NeuvilletteParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::neuvillette4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser. Transcribed from the prototype
        // (Tools/Misc/Prototypes/neuvilletteMelusentFix.py), confirmed in game on seven mods.
        //
        // THREE drawn objects off one mesh: head (first index 0), body (33879), dress (79377). His head and
        // dress draw on the PLAIN layout (ps-t0 diffuse, ps-t1 light map), his body on the normal-map one
        // (ps-t0 normal map, ps-t1 diffuse, ps-t2 light map), off his own frame dump. His Texcoord is
        // stride 20: Data/Mod Downloads/GI/Neuvillette/4_0's Texcoord.buf, 495240 bytes / 24762 vertices.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Neuvillette;
        config.downloadCharFolder = "Neuvillette";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Neuvillette";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        // HIS MODS DO NOT AGREE ON A LAYOUT: one writes his head on the normal-map layout (diffuse ps-t1,
        // light map ps-t2, ORFix) where another writes it plain, and a download keyed on ps-t0 fired on the
        // first and put his head DIFFUSE in the normal-map slot -- a hair ribbon drawn flat green
        // (2026-09-24). A section binding ANY texture register brings its own set: see
        // GIMICharParserConfig::ObjDownloadRegs::coverRegs.
        const std::vector<std::string> cover = {"ps-t0", "ps-t1", "ps-t2"};
        config.objDownloadRegs = {{"head", "ps-t0", "ps-t1", "", cover},
                                  {"body", "ps-t1", "ps-t2", "ps-t0", cover},
                                  {"dress", "ps-t0", "ps-t1", "", cover}};

        // No face download. The face meshes are shared with the skin, so a face section exists only because
        // a mod overrides the face: the download could only ever fire wrongly. See
        // GIMICharParserConfig::faceDownload.
        config.faceDownload = false;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory NeuvilletteParser::v4_0() {
        return IniParseBuilderFuncs::neuvillette4_0();
    }
}
