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

#include "AGRemapCore/data/IniParseData/Lumine/LumineParser.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::lumine4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser. Transcribed from the prototype
        // (Tools/Misc/Prototypes/lumineHeavenFix.py), confirmed in game on her identity mod and ten real ones.
        //
        // THREE drawn objects off one mesh: head (first index 0), body (6915), dress (40413), all on the PLAIN layout
        // (ps-t0 diffuse, ps-t1 light map) off her own frame dump -- she has no normal map. Her Texcoord is stride 20:
        // Data/Mod Downloads/GI/Lumine/4_0's Texcoord.buf, 240440 bytes / 12022 vertices.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Lumine;
        config.downloadCharFolder = "Lumine";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Lumine";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        // A section binding ANY texture register brings its own set -- see
        // GIMICharParserConfig::ObjDownloadRegs::coverRegs (Neuvillette's lesson): two of her mods are the older GIMI
        // shape with a MetalMap / ShadowRamp at ps-t2 / ps-t3.
        const std::vector<std::string> cover = {"ps-t0", "ps-t1", "ps-t2"};
        config.objDownloadRegs = {{"head", "ps-t0", "ps-t1", "", cover},
                                  {"body", "ps-t0", "ps-t1", "", cover},
                                  {"dress", "ps-t0", "ps-t1", "", cover}};

        // No face download: a face section exists only because a mod overrides the face, and the skin draws its OWN
        // face, which this fix leaves alone (see LumineFixer). See GIMICharParserConfig::faceDownload.
        config.faceDownload = false;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory LumineParser::v4_0() {
        return IniParseBuilderFuncs::lumine4_0();
    }
}
