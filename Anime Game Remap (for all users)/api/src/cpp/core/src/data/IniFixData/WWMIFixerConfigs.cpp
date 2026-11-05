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

#include <cctype>
#include <map>

#include "AGRemapCore/constants/IniKeywords.h"

#include "AGRemapCore/data/IniFixData/Changli/ChangliFixer.h"
#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphFixer.h"
#include "AGRemapCore/data/IniFixData/Chisa/ChisaFixer.h"
#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitFixer.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeFixer.h"
#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintFixer.h"
#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaFixer.h"
#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistFixer.h"


namespace AGRemapCore {
    std::string WWMIFixerConfig::roleKind(const std::string& role) {
        for (std::size_t i = 0; i < role.size(); ++i) {
            if (std::isupper(static_cast<unsigned char>(role[i])) != 0) {
                return role.substr(i);
            }
        }
        return "";
    }

    std::optional<std::string> WWMIFixerConfig::registerOf(const std::vector<Binding>& bindings, const std::string& role) {
        std::map<std::string, std::string> regOfRole;
        for (const Binding& binding : bindings) {
            if (binding.role != IniKeywords::Null) {
                regOfRole.emplace(binding.role, binding.reg);
            }
        }
        if (const auto found = regOfRole.find(role); found != regOfRole.end()) {
            return found->second;
        }

        const std::string kind = roleKind(role);
        std::optional<std::string> byKind;
        for (const auto& [rowRole, reg] : regOfRole) {
            if (kind.empty() || roleKind(rowRole) != kind) {
                continue;
            }
            if (byKind.has_value() && *byKind != reg) {
                return std::nullopt;
            }
            byKind = reg;
        }
        return byKind;
    }

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
