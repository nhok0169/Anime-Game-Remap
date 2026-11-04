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

#include "AGRemapCore/data/IniParseData/WWMIParser.h"

#include "AGRemapCore/data/IniParseData/Changli/ChangliParser.h"
#include "AGRemapCore/data/IniParseData/ChangliNymph/ChangliNymphParser.h"
#include "AGRemapCore/data/IniParseData/Chisa/ChisaParser.h"
#include "AGRemapCore/data/IniParseData/ChisaParfait/ChisaParfaitParser.h"
#include "AGRemapCore/data/IniParseData/Lynae/LynaeParser.h"
#include "AGRemapCore/data/IniParseData/LynaePeppermint/LynaePeppermintParser.h"
#include "AGRemapCore/data/IniParseData/Sanhua/SanhuaParser.h"
#include "AGRemapCore/data/IniParseData/SanhuaExorcist/SanhuaExorcistParser.h"


namespace AGRemapCore {
    std::optional<WWMIParserConfig> WWMIParserConfig::of(ModTypeId id) {
        switch (id) {
            case ModTypeId::Changli: return ChangliParser::v1_1Config();
            case ModTypeId::ChangliNymph: return ChangliNymphParser::v2_4Config();
            case ModTypeId::Chisa: return ChisaParser::v2_8Config();
            case ModTypeId::ChisaParfait: return ChisaParfaitParser::v3_5Config();
            case ModTypeId::Lynae: return LynaeParser::v3_6Config();
            case ModTypeId::LynaePeppermint: return LynaePeppermintParser::v3_7Config();
            case ModTypeId::Sanhua: return SanhuaParser::v2_5Config();
            case ModTypeId::SanhuaExorcist: return SanhuaExorcistParser::v2_5Config();
            default: return std::nullopt;
        }
    }
}
