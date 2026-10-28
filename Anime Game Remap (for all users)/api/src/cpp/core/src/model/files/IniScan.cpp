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


#include "AGRemapCore/model/files/IniScan.h"

#include <fstream>
#include <istream>
#include <sstream>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {

    namespace {
        std::vector<IniScanSection> scanStream(std::istream& in) {
            std::vector<IniScanSection> sections;
            std::string line;
            while (std::getline(in, line)) {
                std::string stripped = std::string(StringTools::strip(line));
                if (stripped.empty() || stripped[0] == ';') {
                    continue;
                }

                // IniFile's header rule, which is 3DMigoto's: '[' opens a section, closed or not.
                if (stripped.front() == '[') {
                    sections.push_back(IniScanSection{IniFile::getSectionNameFromLine(stripped), {}});
                    continue;
                }

                if (sections.empty()) {
                    continue;
                }

                sections.back().lines.push_back(stripped);

                const std::size_t eq = stripped.find('=');
                if (eq == std::string::npos) {
                    continue;
                }

                sections.back().kvps.emplace_back(std::string(StringTools::strip(stripped.substr(0, eq))),
                                                  std::string(StringTools::strip(stripped.substr(eq + 1))));
            }

            return sections;
        }
    }


    std::vector<IniScanSection> IniScan::scan(const std::string& path) {
        std::ifstream in(FileService::strToPath(path));
        return scanStream(in);
    }


    std::vector<IniScanSection> IniScan::scanText(const std::string& text) {
        std::istringstream in(text);
        return scanStream(in);
    }


    std::optional<std::string> IniScan::firstVal(const IniScanSection& section, const std::string& key) {
        for (const auto& kvp : section.kvps) {
            if (kvp.first == key) {
                return kvp.second;
            }
        }

        return std::nullopt;
    }
}
