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

#include "AGRemapCore/data/IniParseData/Nilou/NilouParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    namespace {
        // Everything the two versions agree on, which is everything except where the downloaded
        // textures hang off.
        GIMICharParserConfig nilouBase() {
            GIMICharParserConfig config{};
            config.modTypeId = ModTypeId::Nilou;
            config.downloadCharFolder = "Nilou";

            // 4_0 for BOTH versions. The pure-Python 5.7 row reaches back into
            // FileDownloadData[4.0] for its buffers and its 5.7 entry points at the same folder --
            // the assets did not change, only which register the .ini binds them to.
            config.downloadVersionFolder = "4_0";
            config.downloadPrefix = "Nilou";
            config.drawnObjs = {"head", "body", "dress"};
            config.texcoordStride = 20;

            // Her face diffuse sits under 5_4 while the rest of her assets are in 4_0. The spelling
            // is unchanged, unlike AyakaSpringbloom's.
            config.faceDownloadVersionFolder = "5_4";

            return config;
        }
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::nilou4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Nilou does differently lives here.
        GIMICharParserConfig config = nilouBase();

        // A 4.0-ERA SHADER READS ITS DIFFUSE A SLOT HIGHER. All three objects here put the diffuse
        // on ps-t1 and the lightmap on ps-t2, leaving ps-t0 for the normal map the model carried.
        // The 5.7 row below is the same character after GI moved them down; a download on the wrong
        // slot is sampled as the wrong kind of texture, and nothing reports it.
        config.objDownloadRegs = {{"head", "ps-t1", "ps-t2"},
                                  {"body", "ps-t1", "ps-t2"},
                                  {"dress", "ps-t1", "ps-t2"}};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::nilou5_7() {
        // Identical to 4.0 except that the diffuse and lightmap have moved to ps-t0/ps-t1, which
        // is makeGIMICharParser's own default -- so this row names no registers at all.
        return makeGIMICharParser(nilouBase());
    }


    IniParseBuilder::Factory NilouParser::v4_0() {
        return IniParseBuilderFuncs::nilou4_0();
    }


    IniParseBuilder::Factory NilouParser::v5_7() {
        return IniParseBuilderFuncs::nilou5_7();
    }
}
