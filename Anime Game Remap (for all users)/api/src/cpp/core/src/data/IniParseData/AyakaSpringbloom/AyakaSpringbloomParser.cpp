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

#include "AGRemapCore/data/IniParseData/AyakaSpringbloom/AyakaSpringbloomParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    namespace {
        GIMICharParserConfig springbloomBase() {
            GIMICharParserConfig config{};
            config.modTypeId = ModTypeId::AyakaSpringbloom;

            // "AyakaSpringbloom" here (the download FOLDER) but "AyakaSpringBloom" for her assets --
            // ModTypeIdTools::getName returns the capital-B spelling, mirroring ModTypeNames.py. The
            // two really do differ; see ModTypeId.cpp's own note.
            config.downloadCharFolder = "AyakaSpringbloom";
            config.downloadVersionFolder = "4_0";
            config.downloadPrefix = "AyakaSpringBloom";
            config.drawnObjs = {"head", "body", "dress"};
            config.texcoordStride = 20;

            // HER FACE DIFFUSE IS FILED DIFFERENTLY FROM EVERYTHING ELSE SHE HAS: under 5_4 rather
            // than 4_0, and spelled with the folder's lowercase 'b' where every other file of hers
            // uses AyakaSpringBloom. Both halves matter -- a raw GitHub URL is case-sensitive.
            config.faceDownloadVersionFolder = "5_4";
            config.faceDownloadPrefix = "AyakaSpringbloom";

            return config;
        }

        // Her head and body sit a slot higher through 5.6; only the dress is already on the modern
        // layout. See NilouParser for what that is about.
        const GIMICharParserConfig::ObjDownloadRegs OldLayout(const std::string& obj) {
            return {obj, "ps-t1", "ps-t2"};
        }
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::ayakaSpringbloom4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what AyakaSpringbloom does differently lives here.
        GIMICharParserConfig config = springbloomBase();
        config.objDownloadRegs = {OldLayout("head"), OldLayout("body")};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::ayakaSpringbloom5_6() {
        // Same downloads as 4.0. The version exists because her TEXTURE EDITS changed, and those
        // live on the fixer side -- see AyakaSpringbloomFixer.
        GIMICharParserConfig config = springbloomBase();
        config.objDownloadRegs = {OldLayout("head"), OldLayout("body")};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::ayakaSpringbloom5_7() {
        // By 5.7 her head and body have moved down to ps-t0/ps-t1 -- the default -- so this row
        // names no registers at all.
        return makeGIMICharParser(springbloomBase());
    }


    IniParseBuilder::Factory AyakaSpringbloomParser::v4_0() {
        return IniParseBuilderFuncs::ayakaSpringbloom4_0();
    }


    IniParseBuilder::Factory AyakaSpringbloomParser::v5_6() {
        return IniParseBuilderFuncs::ayakaSpringbloom5_6();
    }


    IniParseBuilder::Factory AyakaSpringbloomParser::v5_7() {
        return IniParseBuilderFuncs::ayakaSpringbloom5_7();
    }
}
