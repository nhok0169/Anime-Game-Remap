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

#include "AGRemapCore/data/IniParseData/Citlali/CitlaliParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::citlali5_3() {
        // The standard GIMI character shape -- see makeGIMICharParser. Only what Citlali does
        // differently lives here.
        //
        // TWO drawn objects, head and body, BOTH on the normal-map texture layout: ps-t0 normal
        // map, ps-t1 diffuse, ps-t2 light map, read off her own frame dump (the head and body draws
        // bind LND on shader 2c157719180b096c) and her identity mod. Her eyes are in the BODY
        // object (vertex groups 5 / 6).
        //
        // Her Texcoord is stride 20 -- a second UV set, all zero -- off Data/Mod Downloads/GI/
        // Citlali/5_3's Texcoord.buf (542040 bytes / 27102 vertices).
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Citlali;
        config.downloadCharFolder = "Citlali";
        config.downloadVersionFolder = "5_3";
        config.downloadPrefix = "Citlali";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 20;
        config.objDownloadRegs = {{"head", "ps-t1", "ps-t2", "ps-t0"},
                                  {"body", "ps-t1", "ps-t2", "ps-t0"}};

        // No face download. The skin's face and base Citlali's are the SAME mesh by the same hashes
        // and both read the diffuse at ps-t1 (GI 6.x), so a mod made from a current dump binds its
        // face there and the ps-t0-only download would fetch a texture the mod already has -- and
        // Data/Mod Downloads/GI/Citlali is not on the branch the downloader reads yet, so the fetch
        // 404s and leaves a dangling reference. See GIMICharParserConfig::faceDownload.
        config.faceDownload = false;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory CitlaliParser::v5_3() {
        return IniParseBuilderFuncs::citlali5_3();
    }
}
