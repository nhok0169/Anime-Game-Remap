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

#include "AGRemapCore/data/IniFixData/RegValChecks.h"

#include <algorithm>
#include <cctype>


namespace AGRemapCore {

    namespace {
        // Lowercased ASCII, then a plain substring search -- the pure-Python original is
        // val.lower().find(needle) != -1 and this is value-for-value the same. 'needle' is always
        // an ASCII literal from this file, so there is nothing here for a grapheme-aware compare
        // to do.
        bool containsIgnoreCase(const std::string& val, const char* needle) {
            std::string lowered;
            lowered.reserve(val.size());

            for (const char c : val) {
                lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }

            return lowered.find(needle) != std::string::npos;
        }
    }

    bool RegValChecks::isDiffuse(const std::string& val) {
        return containsIgnoreCase(val, "diffuse");
    }

    bool RegValChecks::isLightMap(const std::string& val) {
        return containsIgnoreCase(val, "lightmap");
    }

    bool RegValChecks::isNormalMap(const std::string& val) {
        return containsIgnoreCase(val, "normalmap");
    }

    bool RegValChecks::isMetalMap(const std::string& val) {
        return containsIgnoreCase(val, "metalmap");
    }

    bool RegValChecks::isShadow(const std::string& val) {
        return containsIgnoreCase(val, "shadow");
    }
}
