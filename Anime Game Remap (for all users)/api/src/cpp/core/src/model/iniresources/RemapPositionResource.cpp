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

#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/model/iniresources/RemapPositionResource.h"

#include <utility>
#include <vector>

#include "AGRemapCore/model/files/PositionFile.h"


namespace AGRemapCore {

    RemapPositionResource::RemapPositionResource(const std::string& iniFolderPath, const std::string& srcPath,
                                                  const std::string& fixedPath, BufFile::Filter edit,
                                                  std::string type,
                                                  std::function<bool(RemapPositionResource&)> fixFunc):
        RemapIniFixResource(std::move(type), iniFolderPath, srcPath, fixedPath),
        edit(std::move(edit)), fixFunc(std::move(fixFunc)) {}


    bool RemapPositionResource::srcEncounteredError(const RemapStats& stats) const {
        return stats.position.skipped.contains(srcPath);
    }

    bool RemapPositionResource::srcIsFixed(const RemapStats& stats) const {
        return stats.position.fixed.contains(srcPath);
    }

    bool RemapPositionResource::fixEncounteredError(const RemapStats& stats) const {
        return stats.position.skipped.contains(fixedPath);
    }

    bool RemapPositionResource::fixIsFixed(const RemapStats& stats) const {
        return stats.position.fixed.contains(fixedPath);
    }


    bool RemapPositionResource::_fix() {
        // No edit is not an error, it is "nothing to do" -- and writing a byte-for-byte copy under a
        // remapped name would be worse than doing nothing, since the copy then goes stale silently
        // if the original is ever updated.
        if (!edit) {
            return false;
        }

        PositionFile position(srcPath);
        position.fix(fixedPath, std::vector<BufFile::Filter>{edit});
        return true;
    }


    bool RemapPositionResource::fix() {
        // Before the work, not after: this is the line that explains a long pause, and the one still
        // on screen if the edit below throws. Same shape as RemapBlendResource::fix.
        if (logger != nullptr) {
            logger->log("Fixing position for "
                         + FileService::pathToStr(FileService::strToPath(fixedPath).filename()) + "...");
        }

        if (fixFunc) {
            return fixFunc(*this);
        }

        return _fix();
    }
}
