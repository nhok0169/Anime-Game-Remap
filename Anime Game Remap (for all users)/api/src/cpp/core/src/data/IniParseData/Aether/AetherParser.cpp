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

#include "AGRemapCore/data/IniParseData/Aether/AetherParser.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::aether4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser. Transcribed from the prototype
        // (Tools/Misc/Prototypes/aetherHeavenFix.py), checked in game on his identity mod and ten real ones.
        //
        // TWO drawn objects off one mesh: head (first index 0) and body (8874), both on the PLAIN layout (ps-t0
        // diffuse, ps-t1 light map) off his own frame dump -- he has no normal map. His Texcoord is stride 12:
        // Data/Mod Downloads/GI/Aether/4_0's Texcoord.buf, 154752 bytes / 12896 vertices.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Aether;
        config.downloadCharFolder = "Aether";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Aether";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        // A section binding ANY texture register brings its own set -- see
        // GIMICharParserConfig::ObjDownloadRegs::coverRegs: an older GIMI mod binds a MetalMap / ShadowRamp at
        // ps-t2 / ps-t3 beside its diffuse and light map.
        const std::vector<std::string> cover = {"ps-t0", "ps-t1", "ps-t2"};
        config.objDownloadRegs = {{"head", "ps-t0", "ps-t1", "", cover},
                                  {"body", "ps-t0", "ps-t1", "", cover}};

        // No face download: a face section exists only because a mod overrides the face, and the skin draws its OWN
        // face, which this fix leaves alone (see AetherFixer). See GIMICharParserConfig::faceDownload.
        config.faceDownload = false;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory AetherParser::v4_0() {
        return IniParseBuilderFuncs::aether4_0();
    }
}
