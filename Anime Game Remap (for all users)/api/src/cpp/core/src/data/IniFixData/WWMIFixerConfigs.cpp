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

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"

#include "AGRemapCore/data/IniFixData/Changli/ChangliFixer.h"
#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphFixer.h"
#include "AGRemapCore/data/IniFixData/Chisa/ChisaFixer.h"
#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitFixer.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeFixer.h"
#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintFixer.h"
#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaFixer.h"
#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistFixer.h"


namespace AGRemapCore {
    std::optional<WWMIFixerConfig> WWMIFixerConfig::of(ModTypeId source, ModTypeId target) {
        if (source == ModTypeId::Changli && target == ModTypeId::ChangliNymph) {
            return ChangliFixer::nymph2_4Config();
        }
        if (source == ModTypeId::ChangliNymph && target == ModTypeId::Changli) {
            return ChangliNymphFixer::changli1_4Config();
        }
        if (source == ModTypeId::Chisa && target == ModTypeId::ChisaParfait) {
            return ChisaFixer::parfait3_7Config();
        }
        if (source == ModTypeId::ChisaParfait && target == ModTypeId::Chisa) {
            return ChisaParfaitFixer::chisa2_8Config();
        }
        if (source == ModTypeId::Lynae && target == ModTypeId::LynaePeppermint) {
            return LynaeFixer::peppermint3_7Config();
        }
        if (source == ModTypeId::LynaePeppermint && target == ModTypeId::Lynae) {
            return LynaePeppermintFixer::lynae3_7Config();
        }
        if (source == ModTypeId::Sanhua && target == ModTypeId::SanhuaExorcist) {
            return SanhuaFixer::exorcist2_5Config();
        }
        if (source == ModTypeId::SanhuaExorcist && target == ModTypeId::Sanhua) {
            return SanhuaExorcistFixer::sanhua2_5Config();
        }
        return std::nullopt;
    }
}
