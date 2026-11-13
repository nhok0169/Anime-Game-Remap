#include "AGRemapCore/data/IniParseData/Lynae/LynaeParser.h"

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

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeTextures.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeFilmShader.h"


namespace AGRemapCore {
    WWMIParserConfig LynaeParser::v3_6Config() {
        // The WWMI shape -- see makeWWMIParser. Her eight draw slots come off the library's Indices
        // rows, her hashes off HashData; 3.6 is the vb0 most of her mods carry.
        WWMIParserConfig config{};
        config.version = "3.6";
        // Her own textures, so a mod of hers can be sorted into roles at PARSE time
        config.textures = lynaeTextureFacts();
        // the numberings her skeleton has had and her own bones' centroids: read off VGRemaps and BoneCentroids
        // (WWMIParserConfig::withGameData) -- see WWMIParserConfig::skeletonNumberings
        // the draws LynaeFixer's drawReroutes send elsewhere: her jacket's and props' film (Lynae9 draws its props
        // with the game's code map, hence the mask), and any draw of her lower body with a real share of her
        // tights -- Lynae9 draws legs, tights and shorts in one range whose majority is skin (see LynaeFixer). Each
        // flag's bits are her code maps' MaterialCodes rows (WWMIParserConfig::withGameData)
        WWMIParserConfig::DrawSplit jacketFilm{"jacketFilm", 5, "jacketDetail"};
        jacketFilm.flag = "film";
        WWMIParserConfig::DrawSplit propsFilm{"propsFilm", 6, "propsDetail"};
        propsFilm.flag = "film";
        propsFilm.sourceFlags = lynaePropsFilmFlags();
        WWMIParserConfig::DrawSplit lowerTights{"lowerTights", 4, "lowerDetail"};
        lowerTights.flag = "tights";
        lowerTights.share = 0.01;
        config.drawSplits = {std::move(jacketFilm), std::move(propsFilm), std::move(lowerTights)};
        return config;
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::lynae3_6() {
        return makeWWMIParser(LynaeParser::v3_6Config());
    }


    IniParseBuilder::Factory LynaeParser::v3_6() {
        return IniParseBuilderFuncs::lynae3_6();
    }
}
