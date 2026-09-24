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

#include "AGRemapCore/data/IniParseData/Charlotte/CharlotteParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::charlotte4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser. Only what Charlotte does
        // differently lives here.
        //
        // TWO drawn objects, head and body, BOTH on the normal-map texture layout: ps-t0 normal map,
        // ps-t1 diffuse, ps-t2 light map, read off her own 6.7 frame dump (the head and body draws
        // bind LND on shader 2c157719180b096c) and her identity mod. Her eyes are in the HEAD object
        // (vertex groups 13 / 14).
        //
        // Her Texcoord is stride 12, off Data/Mod Downloads/GI/Charlotte/4_0's Texcoord.buf
        // (213480 bytes / 17790 vertices).
        //
        // Her CAMERA is a separate mesh (ib deb75778) her hash.json does not list, and an accessory
        // rather than part of her (the maintainer, 2026-09-23): nothing here classifies it.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Charlotte;
        config.downloadCharFolder = "Charlotte";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Charlotte";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;
        config.objDownloadRegs = {{"head", "ps-t1", "ps-t2", "ps-t0"},
                                  {"body", "ps-t1", "ps-t2", "ps-t0"}};

        // No face download. The skin's face and base Charlotte's are the SAME mesh by the same hashes
        // and the same diffuse (58d9859b), both read at ps-t1 (GI 6.x), so a face section only exists
        // because a mod overrides the face, and the ps-t0-only download could only ever fire wrongly.
        // See GIMICharParserConfig::faceDownload.
        config.faceDownload = false;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory CharlotteParser::v4_0() {
        return IniParseBuilderFuncs::charlotte4_0();
    }
}
