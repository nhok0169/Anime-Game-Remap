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

#include "AGRemapCore/model/iniresources/IniSrcResourceModel.h"

#include <utility>

#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    IniSrcResourceModel::IniSrcResourceModel(std::string iniFolderPath, tsl::ordered_map<int, std::vector<std::string>> paths):
        IniResourceModel(std::move(iniFolderPath)), paths(std::move(paths)) {
        for (const auto& entry : this->paths) {
            std::vector<std::string> resolved;
            resolved.reserve(entry.second.size());

            for (const std::string& path : entry.second) {
                resolved.push_back(FileService::absPathOfRelPath(path, this->iniFolderPath));
            }

            fullPaths.emplace(entry.first, std::move(resolved));
        }
    }

    std::vector<std::pair<std::string, std::string>> IniSrcResourceModel::items() const {
        std::vector<std::pair<std::string, std::string>> result;

        for (const auto& entry : paths) {
            const std::vector<std::string>& partPaths = entry.second;
            const std::vector<std::string>& partFullPaths = fullPaths.at(entry.first);

            for (size_t i = 0; i < partPaths.size(); ++i) {
                result.emplace_back(partPaths[i], partFullPaths[i]);
            }
        }

        return result;
    }
}
