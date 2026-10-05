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

#include "AGRemapCore/data/IniFixData/SideMeshes.h"

#include <algorithm>
#include <string_view>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/tools/StringTools.h"

namespace AGRemapCore {
    namespace {
        struct Section {
            std::string name;
            std::vector<std::string> body;
        };

        // The file's sections in order, each with the lines under its header (carriage returns dropped). A line
        // before the first header belongs to none. Byte-wise on the ASCII delimiters '\n', '[' and '='.
        std::vector<Section> sectionsOf(const std::string& fileTxt) {
            std::vector<Section> sections;
            for (std::size_t at = 0; at <= fileTxt.size();) {
                std::size_t end = fileTxt.find('\n', at);
                if (end == std::string::npos) {
                    end = fileTxt.size();
                }
                std::string line = fileTxt.substr(at, end - at);
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                at = end + 1;

                const std::string_view stripped = StringTools::strip(line);
                if (!stripped.empty() && stripped.front() == '[') {
                    const std::size_t close = stripped.find(']');
                    sections.push_back(Section{std::string(stripped.substr(1, (close == std::string_view::npos) ? std::string_view::npos : close - 1)), {}});
                } else if (!sections.empty()) {
                    sections.back().body.push_back(std::move(line));
                }
            }
            return sections;
        }

        bool isHashLine(const std::string& line) {
            const std::size_t eq = line.find('=');
            return eq != std::string::npos
                && StringTools::equalsIgnoreCase(StringTools::strip(std::string_view(line).substr(0, eq)), IniKeywords::Hash);
        }
    }


    std::string SideMeshes::build(const std::string& fileTxt, Hashes& hashes, const std::string& srcName,
                                  const std::optional<Version>& fromVersion, const std::vector<std::string>& types,
                                  const std::string& targetName, const std::optional<Version>& toVersion) {
        if (types.empty()) {
            return "";
        }

        std::string out;
        for (Section& section : sectionsOf(fileTxt)) {
            auto hashLine = std::find_if(section.body.begin(), section.body.end(), isHashLine);
            if (hashLine == section.body.end()) {
                continue;
            }

            const std::string from = StringTools::toLower(std::string(StringTools::strip(
                std::string_view(*hashLine).substr(hashLine->find('=') + 1))));
            const std::optional<std::vector<std::string>> key =
                hashes.getKey(from, fromVersion, std::vector<std::optional<std::string>>{srcName, std::nullopt}, false);
            if (!key.has_value() || key->empty() || std::find(types.begin(), types.end(), key->back()) == types.end()) {
                continue;
            }

            const std::optional<std::string> to = hashes.get({targetName, key->back()}, toVersion, false);
            if (!to.has_value() || to->empty() || StringTools::equalsIgnoreCase(from, *to)) {
                continue;
            }

            // Less its trailing blank and comment lines, which belong to whatever follows.
            while (!section.body.empty()) {
                const std::string_view last = StringTools::strip(section.body.back());
                if (!last.empty() && last.front() != ';') {
                    break;
                }
                section.body.pop_back();
            }

            out += "\n[" + section.name + targetName + IniKeywords::Remap + "]\n";
            for (const std::string& line : section.body) {
                out += (isHashLine(line) ? IniKeywords::Hash + " = " + *to : line) + "\n";
            }
        }

        if (out.empty()) {
            return "";
        }

        return "; The mod's own sections on " + srcName + "'s side meshes (face, head-upper, ...), on\n"
               "; " + targetName + "'s: it draws its own under other hashes, which the mod's sections do not reach.\n"
               + out;
    }
}
