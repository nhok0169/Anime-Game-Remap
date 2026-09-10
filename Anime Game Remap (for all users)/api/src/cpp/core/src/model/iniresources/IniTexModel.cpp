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

#include "AGRemapCore/model/iniresources/IniTexModel.h"

#include <utility>


namespace AGRemapCore {
    IniTexModel::IniTexModel(std::string iniFolderPath,
                              tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fixedPaths,
                              tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::unique_ptr<BaseTexEditor>>>> texEdits,
                              std::optional<tsl::ordered_map<int, std::vector<std::string>>> origPaths):
        IniFixResourceModel(std::move(iniFolderPath), std::move(fixedPaths), std::move(origPaths)),
        texEdits(std::move(texEdits)) {}

    void IniTexModel::clear() {
        IniFixResourceModel::clear();
        texEdits.clear();
    }
}
