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

#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"


namespace AGRemapCore {
    IniClassifyStats BaseIniClassifier::classify(const std::string& iniTxt, GameTypeIdFilter gameTypeIds) {
        return IniClassifyStats();
    }

    IniClassifyStats BaseIniClassifier::classify(const std::vector<std::string>& iniTxt, GameTypeIdFilter gameTypeIds) {
        return IniClassifyStats();
    }

    bool BaseIniClassifier::checkIsMod(const std::string& iniTxt, GameTypeIdFilter gameTypeIds) {
        return false;
    }

    bool BaseIniClassifier::checkIsMod(const std::vector<std::string>& iniTxt, GameTypeIdFilter gameTypeIds) {
        return false;
    }

    void BaseIniClassifier::checkIsFixedMod(const std::string& iniTxt, bool* isFixed, bool* isMod, GameTypeIdFilter gameTypeIds) {
        *isFixed = false;
        *isMod = false;
    }

    void BaseIniClassifier::checkIsFixedMod(const std::vector<std::string>& iniTxt, bool* isFixed, bool* isMod, GameTypeIdFilter gameTypeIds) {
        *isFixed = false;
        *isMod = false;
    }

    void BaseIniClassifier::clear() {
        // TODO: filled in later
    }
}
