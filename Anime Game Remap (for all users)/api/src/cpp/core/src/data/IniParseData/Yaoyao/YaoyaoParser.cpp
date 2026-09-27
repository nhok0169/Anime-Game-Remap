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

#include "AGRemapCore/data/IniParseData/Yaoyao/YaoyaoParser.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::yaoyao4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser. Transcribed from the prototype
        // (Tools/Misc/Prototypes/yaoyaoBambooFix.py), confirmed in game on her identity mod and ten real ones.
        //
        // TWO drawn objects off one mesh: head (first index 0), body (21678), both on the PLAIN layout (ps-t0
        // diffuse, ps-t1 light map) off her own frame dump -- she has no normal map. Her Texcoord is stride 12:
        // Data/Mod Downloads/GI/Yaoyao/4_0's Texcoord.buf, 201468 bytes / 16789 vertices.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Yaoyao;
        config.downloadCharFolder = "Yaoyao";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Yaoyao";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        // Her mods do not agree on a layout either: some bind a normal map at ps-t0 with ORFix, some bind a
        // third texture at ps-t2 beside NNFix. A section binding ANY texture register brings its own set --
        // see GIMICharParserConfig::ObjDownloadRegs::coverRegs (Neuvillette's lesson).
        const std::vector<std::string> cover = {"ps-t0", "ps-t1", "ps-t2"};
        config.objDownloadRegs = {{"head", "ps-t0", "ps-t1", "", cover},
                                  {"body", "ps-t0", "ps-t1", "", cover}};

        // No face download. The face meshes are shared with the skin -- under the same hashes and the same
        // face diffuse -- so a face section exists only because a mod overrides the face: the download could
        // only ever fire wrongly. See GIMICharParserConfig::faceDownload.
        config.faceDownload = false;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory YaoyaoParser::v4_0() {
        return IniParseBuilderFuncs::yaoyao4_0();
    }
}
