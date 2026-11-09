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

#include "AGRemapCore/tools/StringTools.h"

#include <algorithm>
#include <exception>
#include "AGRemapCore/data/IniParseData/WWMIParser.h"



namespace AGRemapCore {
    std::optional<int> WWMIParserConfig::slotOf(const std::pair<std::string, std::string>& obj) const {
        if (!obj.first.empty() || !StringTools::startsWith(obj.second, slotPrefix)) {
            return std::nullopt;
        }

        try {
            return std::stoi(obj.second.substr(slotPrefix.size()));
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

}
