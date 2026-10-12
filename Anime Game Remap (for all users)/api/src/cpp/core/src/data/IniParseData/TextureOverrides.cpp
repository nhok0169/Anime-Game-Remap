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
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
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
            if (!hash.has_value()) {
                continue;
            }

            // The first `this =`, with the `if` blocks around it: a recolour behind a toggle
            // (`if $color == 0`) is only the mod's while the toggle says so
            std::vector<std::vector<std::string>> levels;
            std::optional<std::string> resource;
            std::vector<std::vector<std::string>> branches;
            for (const auto& part : entry.second->parts()) {
                if (const auto* pred = dynamic_cast<const IfPredPart*>(part.get())) {
                    const std::string header(StringTools::strip(pred->src));
                    if (pred->type == IfPredPartType::If) {
                        levels.push_back({header});
                    } else if (pred->type == IfPredPartType::EndIf) {
                        if (!levels.empty()) {
                            levels.pop_back();
                        }
                    } else if (!levels.empty()) {
                        levels.back().push_back(header);
                    }
                    continue;
                }

                const auto* content = dynamic_cast<const ModBranches::Template::ContentPart*>(part.get());
                if (content == nullptr) {
                    continue;
                }
                const std::vector<std::string> vals = content->getVals(ThisKey);
                if (!vals.empty()) {
                    resource = std::string(StringTools::strip(vals.front()));
                    branches = levels;
                    break;
                }
            }
            if (!resource.has_value()) {
                continue;
            }

            result.push_back({entry.first, StringTools::toLower(StringTools::strip(*hash)), *resource, std::move(branches)});
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
    namespace {
        // The resource or section names a value can hold: `ref X`, `X`, `X, Y`.
        std::vector<std::string> valueTokens(const std::string& value) {
            std::vector<std::string> tokens;
            std::string current;
            for (char c : value + " ") {
                if (c == ' ' || c == '\t' || c == ',' || c == '=') {
                    if (!current.empty() && current != "ref" && current != "reference") {
                        tokens.push_back(current);
                    }
                    current.clear();
                } else {
                    current += c;
                }
            }
            return tokens;
        }

        // The body lines of one section, read from the file's own text.
        std::string sectionText(IniFile& file, const std::string& name) {
            std::string out;
            bool inside = false;
            for (const std::string& raw : file.readFileLines()) {
                std::string line(StringTools::strip(raw));
                if (line.size() >= 2 && line.front() == '[' && line.back() == ']') {
                    inside = line.substr(1, line.size() - 2) == name;
                    continue;
                }
                if (inside) {
                    out += raw;
                    if (out.empty() || out.back() != '\n') {
                        out += "\n";
                    }
                }
            }
            return out;
        }

        // 'text' with every whole token named in 'renames' replaced.
        std::string renameTokens(const std::string& text, const std::unordered_map<std::string, std::string>& renames) {
            auto isTokenChar = [](char c) { return c != ' ' && c != '\t' && c != ',' && c != '=' && c != '\r' && c != '\n'; };
            std::string out;
            std::size_t i = 0;
            while (i < text.size()) {
                if (!isTokenChar(text[i])) {
                    out += text[i++];
                    continue;
                }
                std::size_t j = i;
                while (j < text.size() && isTokenChar(text[j])) {
                    ++j;
                }
                const std::string token = text.substr(i, j - i);
                auto found = renames.find(token);
                out += (found == renames.end()) ? token : found->second;
                i = j;
            }
            return out;
        }
    }


    std::optional<TextureOverrides::Adoption> TextureOverrides::adopt(IniFile& sibling, const std::string& root,
                                                                      const ModBranches::Templates& own) {
        const ModBranches::Templates& templates = sibling.getIfTemplates();

        // The closure: the section, every sibling section a value names, recursively.
        std::vector<std::string> sections;
        std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> resources;   // sibling resource -> its keys
        std::unordered_set<std::string> seen{root};
        std::vector<std::string> stack{root};
        while (!stack.empty()) {
            const std::string name = stack.back();
            stack.pop_back();
            auto found = templates.find(name);
            if (found == templates.end() || found->second == nullptr) {
                continue;
            }
            const std::optional<std::string> file = ModBranches::firstVal(*found->second, IniKeywords::Filename);
            if (file.has_value()) {
                // Every key, not just the file: an index buffer's `type` and `format` say how the
                // game reads it, and declared without them a 32-bit ib is not read as one.
                std::vector<std::pair<std::string, std::string>> kvps;
                for (const auto& part : found->second->parts()) {
                    const auto* content = dynamic_cast<const IfContentPart<std::string, std::string>*>(part.get());
                    if (content == nullptr) {
                        continue;
                    }
                    for (const auto& kvp : content->entries()) {
                        kvps.emplace_back(kvp.first, std::string(StringTools::strip(kvp.second)));
                    }
                }
                resources.emplace_back(name, std::move(kvps));
                continue;
            }
            sections.push_back(name);
            for (const auto& part : found->second->parts()) {
                const auto* content = dynamic_cast<const IfContentPart<std::string, std::string>*>(part.get());
                if (content == nullptr) {
                    continue;
                }
                for (const auto& kvp : content->entries()) {
                    for (const std::string& token : valueTokens(kvp.second)) {
                        if (templates.find(token) != templates.end() && seen.insert(token).second) {
                            stack.push_back(token);
                        }
                    }
                }
            }
        }

        for (const std::string& name : sections) {
            if (own.find(name) != own.end()) {
                return std::nullopt;
            }
        }

        Adoption result;
        std::unordered_map<std::string, std::string> renames;
        for (const auto& [name, kvps] : resources) {
            renames[name] = name + IniKeywords::RemapRef;
            result.refs.emplace_back(renames[name], kvps);
        }

        std::string text;
        for (const std::string& name : sections) {
            text += "[" + name + "]\n" + renameTokens(sectionText(sibling, name), renames) + "\n";
        }
        IniFile parsed(std::nullopt, text);
        const ModBranches::Templates& parsedTemplates = parsed.getIfTemplates();
        for (const std::string& name : sections) {
            auto found = parsedTemplates.find(name);
            if (found != parsedTemplates.end() && found->second != nullptr) {
                result.sections.emplace_back(name, found->second->deepcopy());
            }
        }
        return result;
    }
}
