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

#include "AGRemapCore/data/IniParseData/TextureOverrides.h"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/HashData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    namespace {
        const std::string ThisKey = "this";
        const std::string TexKeyPrefix = "tex_";
    }


    std::vector<TextureOverrides::Override> TextureOverrides::collect(const ModBranches::Templates& templates) {
        std::vector<Override> result;
        for (const auto& entry : templates) {
            if (entry.second == nullptr) {
                continue;
            }

            const std::optional<std::string> hash = ModBranches::firstVal(*entry.second, IniKeywords::Hash);
            const std::optional<std::string> resource = ModBranches::firstVal(*entry.second, ThisKey);
            if (!hash.has_value() || !resource.has_value()) {
                continue;
            }

            result.push_back({entry.first, StringTools::toLower(StringTools::strip(*hash)),
                              std::string(StringTools::strip(*resource))});
        }
        return result;
    }


    std::vector<std::string> TextureOverrides::siblingInis(IniFile* iniFile) {
        std::vector<std::string> result;
        if (iniFile == nullptr || !iniFile->getFile().has_value()) {
            return result;
        }

        std::error_code error;
        const std::filesystem::path self = FileService::strToPath(*iniFile->getFile());
        for (const auto& entry : std::filesystem::directory_iterator(self.parent_path(), error)) {
            if (!entry.is_regular_file(error) || std::filesystem::equivalent(entry.path(), self, error)) {
                continue;
            }

            const std::string name = FileService::pathToStr(entry.path().filename());
            const std::string low = StringTools::toLower(name);
            if (!StringTools::endsWith(low, ".ini") || StringTools::startsWith(low, "disabled")) {
                continue;
            }

            const std::size_t fix = low.rfind(StringTools::toLower(IniKeywords::RemapFix));
            const std::string stem = low.substr(0, low.size() - 4);
            if (fix != std::string::npos && fix + IniKeywords::RemapFix.size() <= stem.size()
                    && stem.find_first_not_of("0123456789", fix + IniKeywords::RemapFix.size()) == std::string::npos) {
                continue;
            }

            result.push_back(FileService::pathToStr(entry.path()));
        }
        std::sort(result.begin(), result.end());
        return result;
    }


    bool TextureOverrides::isShared(const std::string& hash) {
        // texture hash -> every character HashData files it under, a component folded into its skin.
        // Built once: HashData is compiled in, and the registered mod types are only asked for the
        // component names, which the ModTypeId table fixes.
        static const std::unordered_map<std::string, std::unordered_set<std::string>> owners = []() {
            std::unordered_map<std::string, std::string> skinOf;
            for (const ModType& modType : GlobalModTypes::all()) {
                std::optional<ModTypeId> modTypeId = ModTypeIdTools::getEnum(modType.modTypeId);
                if (!modTypeId.has_value()) {
                    continue;
                }
                for (ModTypeId component : ModTypeIdTools::getComponentIds(*modTypeId)) {
                    skinOf.emplace(ModTypeIdTools::getName(component), modType.name);
                }
            }

            std::unordered_map<std::string, std::unordered_set<std::string>> acc;
            for (const std::pair<std::vector<std::string>, std::string>& row : Data::getHashDataRows()) {
                if (row.first.size() < 3 || !StringTools::startsWith(row.first[2], TexKeyPrefix)) {
                    continue;
                }

                auto skin = skinOf.find(row.first[1]);
                acc[StringTools::toLower(row.second)].insert(skin == skinOf.end() ? row.first[1] : skin->second);
            }
            return acc;
        }();

        auto found = owners.find(StringTools::toLower(StringTools::strip(hash)));
        return found != owners.end() && found->second.size() > 1;
    }


    std::vector<std::string> TextureOverrides::keysOf(const std::string& name, const std::string& hash) {
        // The NEWEST version that files the hash under 'name' decides its labels, as a versioned lookup
        // would: a texture may be relabelled between dumps (LisaStudent's 438c9349 is her head normal
        // map at 4.0 and her head diffuse at 5.4), and one override must not bind at two roles.
        const std::string wanted = StringTools::toLower(StringTools::strip(hash));
        std::optional<Version> newest;
        std::vector<std::string> keys;
        for (const std::pair<std::vector<std::string>, std::string>& row : Data::getHashDataRows()) {
            if (row.first.size() < 3 || row.first[1] != name || !StringTools::startsWith(row.first[2], TexKeyPrefix)
                    || StringTools::toLower(row.second) != wanted) {
                continue;
            }

            const std::optional<Version> parsed = Version::parse(row.first[0]);
            if (!parsed.has_value()) {
                continue;
            }
            const Version& version = *parsed;
            if (!newest.has_value() || *newest < version) {
                newest = version;
                keys.clear();
            } else if (version < *newest) {
                continue;
            }

            if (std::find(keys.begin(), keys.end(), row.first[2]) == keys.end()) {
                keys.push_back(row.first[2]);
            }
        }
        return keys;
    }
}
