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

#include "AGRemapCore/data/WWMITextureFacts.h"
#include "AGRemapCore/constants/WWMINames.h"

#include <algorithm>

#include "AGRemapCore/data/IniFixData/Changli/ChangliTextures.h"
#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphTextures.h"
#include "AGRemapCore/data/IniFixData/Chisa/ChisaTextures.h"
#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitTextures.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeTextures.h"
#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintTextures.h"
#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaTextures.h"
#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistTextures.h"


namespace AGRemapCore {
    std::optional<WWMITextureFacts> WWMITextureFacts::of(ModTypeId id) {
        switch (id) {
            case ModTypeId::Changli: return changliTextureFacts();
            case ModTypeId::ChangliNymph: return changliNymphTextureFacts();
            case ModTypeId::Chisa: return chisaTextureFacts();
            case ModTypeId::ChisaParfait: return chisaParfaitTextureFacts();
            case ModTypeId::Lynae: return lynaeTextureFacts();
            case ModTypeId::LynaePeppermint: return lynaePeppermintTextureFacts();
            case ModTypeId::Sanhua: return sanhuaTextureFacts();
            case ModTypeId::SanhuaExorcist: return sanhuaExorcistTextureFacts();
            default: return std::nullopt;
        }
    }

    std::optional<std::string> WWMITextureFacts::roleOfHashType(const std::string& type, const std::unordered_set<std::string>& roles) {
        if (roles.count(type) > 0) {
            return type;
        }

        for (const std::string& suffix : {WWMINames::LiveSuffix, WWMINames::OldSuffix}) {
            const std::size_t at = type.rfind(suffix);
            if (at == std::string::npos || at == 0) {
                continue;
            }

            const std::string rest = type.substr(at + suffix.size());
            const bool counted = std::all_of(rest.begin(), rest.end(), [](char c) { return c >= '0' && c <= '9'; });
            const std::string role = type.substr(0, at);
            if (counted && (suffix == WWMINames::OldSuffix || rest.empty()) && roles.count(role) > 0) {
                return role;
            }
        }

        return std::nullopt;
    }
}
