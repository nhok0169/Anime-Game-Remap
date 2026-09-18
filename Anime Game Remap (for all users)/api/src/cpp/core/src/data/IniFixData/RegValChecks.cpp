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

#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

    // The pure-Python original is val.lower().find(needle) != -1 -- StringTools::containsIgnoreCase.

    bool RegValChecks::isDiffuse(const std::string& val) {
        return StringTools::containsIgnoreCase(val, "diffuse");
    }

    bool RegValChecks::isLightMap(const std::string& val) {
        return StringTools::containsIgnoreCase(val, "lightmap");
    }

    bool RegValChecks::isNormalMap(const std::string& val) {
        return StringTools::containsIgnoreCase(val, "normalmap");
    }

    bool RegValChecks::isMetalMap(const std::string& val) {
        return StringTools::containsIgnoreCase(val, "metalmap");
    }

    bool RegValChecks::isShadow(const std::string& val) {
        return StringTools::containsIgnoreCase(val, "shadow");
    }
}
