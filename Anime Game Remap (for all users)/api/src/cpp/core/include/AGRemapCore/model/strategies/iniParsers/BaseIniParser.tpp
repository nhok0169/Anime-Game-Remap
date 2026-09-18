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

#ifndef AGRemapCore_BaseIniParser_TPP
#define AGRemapCore_BaseIniParser_TPP

#include "BaseIniParser.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    BaseIniParser<K, V, KeyHash, KeyEqual>::BaseIniParser(IniFile* iniFile): iniFile_(iniFile) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IniFile* BaseIniParser<K, V, KeyHash, KeyEqual>::getIniFile() const {
        return iniFile_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BaseIniParser<K, V, KeyHash, KeyEqual>::setIniFile(IniFile* iniFile) {
        iniFile_ = iniFile;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void BaseIniParser<K, V, KeyHash, KeyEqual>::clear() {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<typename BaseIniParser<K, V, KeyHash, KeyEqual>::GraphGroup> BaseIniParser<K, V, KeyHash, KeyEqual>::parse() {
        return {};
    }
}

#endif
