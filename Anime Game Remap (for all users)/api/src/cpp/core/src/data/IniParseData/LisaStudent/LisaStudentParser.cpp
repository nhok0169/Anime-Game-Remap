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

#include "AGRemapCore/data/IniParseData/LisaStudent/LisaStudentParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    namespace {
        GIMICharParserConfig lisaStudentBase() {
            GIMICharParserConfig config{};
            config.modTypeId = ModTypeId::LisaStudent;
            config.downloadCharFolder = "LisaStudent";
            config.downloadPrefix = "LisaStudent";
            config.drawnObjs = {"head", "body"};
            config.texcoordStride = 20;

            // HER FACE LIVES SOMEWHERE ELSE. Every other character in this batch keeps its face
            // diffuse beside the rest of its assets, but LisaStudent's is only ever published under
            // 5_4 -- `Data/Mod Downloads/GI/LisaStudent/5_4/LisaStudentFaceDiffuse.dds`, with no copy
            // in 4_0. Without this the 4.0 and 5.7 parsers ask for it in their own folder and get a
            // flat 404, which is what happened the first time this batch was run.
            //
            // Same filing quirk as AyakaSpringbloom and Nilou; see faceDownloadVersionFolder.
            config.faceDownloadVersionFolder = "5_4";

            return config;
        }

        // Her diffuse and lightmap sit ONE SLOT HIGHER than the modern layout, because ps-t0 is
        // her normal map. The pure-Python table downloads no normal map for her, so normalMapReg
        // is deliberately left empty -- only the two that are fetched move.
        //
        // HashData's LisaStudent ib row carries the maintainer's own note about this: "Which mf
        // classified ps-t0 as diffuse, in actuality, this a normal map".
        const GIMICharParserConfig::ObjDownloadRegs Shifted(const std::string& obj) {
            return {obj, "ps-t1", "ps-t2"};
        }
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::lisaStudent4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser. Only what LisaStudent does
        // differently lives here.
        GIMICharParserConfig config = lisaStudentBase();
        config.downloadVersionFolder = "4_0";
        config.objDownloadRegs = {Shifted("head"), Shifted("body")};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::lisaStudent5_4() {
        // SAME REGISTERS, DIFFERENT FOLDER. 5.4 re-dumped her assets into their own 5_4 directory
        // while leaving the shifted layout alone -- the same filing quirk AyakaSpringbloom and Nilou
        // have, and a raw GitHub URL is case- and path-sensitive, so pointing at 4_0 here is a 404
        // rather than a near miss.
        GIMICharParserConfig config = lisaStudentBase();
        config.downloadVersionFolder = "5_4";
        config.objDownloadRegs = {Shifted("head"), Shifted("body")};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::lisaStudent5_7() {
        // By 5.7 she is on the modern ps-t0/ps-t1 layout, and the pure-Python row points back at the
        // ORIGINAL 4_0 folder to get there -- so this is the base config with no register override at
        // all. Getting it wrong is silent: the diffuse would be fetched onto the slot her normal map
        // is bound to.
        GIMICharParserConfig config = lisaStudentBase();
        config.downloadVersionFolder = "4_0";

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory LisaStudentParser::v4_0() {
        return IniParseBuilderFuncs::lisaStudent4_0();
    }


    IniParseBuilder::Factory LisaStudentParser::v5_4() {
        return IniParseBuilderFuncs::lisaStudent5_4();
    }


    IniParseBuilder::Factory LisaStudentParser::v5_7() {
        return IniParseBuilderFuncs::lisaStudent5_7();
    }
}
