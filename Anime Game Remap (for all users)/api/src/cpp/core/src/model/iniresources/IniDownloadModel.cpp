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

#include "AGRemapCore/model/iniresources/IniDownloadModel.h"

#include <utility>


namespace AGRemapCore {
    IniDownloadModel::IniDownloadModel(std::string iniFolderPath, tsl::ordered_map<int, std::vector<std::string>> paths,
                                        tsl::ordered_map<int, std::vector<std::unique_ptr<FileDownload>>> downloads):
        IniSrcResourceModel(std::move(iniFolderPath), std::move(paths)), downloads(std::move(downloads)) {}
}
