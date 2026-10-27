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
#include "AGRemapCore/model/iniresources/RemapFileResource.h"

#include <filesystem>
#include <fstream>
#include <utility>

#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/stats/RemapStats.h"


namespace AGRemapCore {
    RemapFileAddResource::RemapFileAddResource(const std::string& iniFolderPath, const std::string& srcPath,
                                               std::vector<std::uint8_t> bytes, std::string type,
                                               std::function<bool(RemapFileAddResource&)> fixFunc):
        RemapIniResource(std::move(type), iniFolderPath, srcPath), bytes(std::move(bytes)), fixFunc(std::move(fixFunc)) {}

    bool RemapFileAddResource::srcEncounteredError(const RemapStats& stats) const {
        return stats.fileAdd.skipped.contains(srcPath);
    }

    bool RemapFileAddResource::srcIsFixed(const RemapStats& stats) const {
        return stats.fileAdd.fixed.contains(srcPath);
    }

    bool RemapFileAddResource::fixEncounteredError(const RemapStats& stats) const {
        return srcEncounteredError(stats);
    }

    bool RemapFileAddResource::fixIsFixed(const RemapStats& stats) const {
        return srcIsFixed(stats);
    }

    bool RemapFileAddResource::fixExists(const RemapStats& stats) const {
        (void)stats;
        return std::filesystem::exists(FileService::strToPath(srcPath));
    }

    bool RemapFileAddResource::_fix() {
        FileService::makeFolderFor(srcPath);
        {
            std::ofstream out(FileService::strToPath(srcPath), std::ios::binary);
            if (!out) {
                return false;
            }
            out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        }

        // a write that failed silently (a full disk, a path the OS refused) leaves a short file
        return FileService::fileSize(srcPath) == static_cast<std::uintmax_t>(bytes.size());
    }

    bool RemapFileAddResource::fix() {
        if (logger != nullptr) {
            logger->log("Creating " + FileService::pathToStr(FileService::strToPath(srcPath).filename()));
        }

        if (fixFunc) {
            return fixFunc(*this);
        }

        return _fix();
    }


    RemapBufRestrideResource::RemapBufRestrideResource(const std::string& iniFolderPath, const std::string& fromPath,
                                                       const std::string& fixedPath,
                                                       std::vector<std::unique_ptr<BufElementType>> fromElements,
                                                       std::vector<std::unique_ptr<BufElementType>> toElements,
                                                       std::unordered_map<std::string, std::string> fillFrom, std::string type):
        RemapFileAddResource(iniFolderPath, fixedPath, {}, std::move(type)),
        fromPath(FileService::absPathOfRelPath(fromPath, iniFolderPath)), fromElements(std::move(fromElements)),
        toElements(std::move(toElements)), fillFrom(std::move(fillFrom)) {}

    bool RemapBufRestrideResource::_fix() {
        std::vector<std::unique_ptr<BufElementType>> elements;
        for (const auto& element : fromElements) {
            elements.push_back(std::make_unique<BufElementType>(*element));
        }
        try {
            BufFile buf(fromPath, std::move(elements));
            bytes = buf.restride(toElements, fillFrom);
        } catch (const std::exception&) {
            return false;
        }
        return RemapFileAddResource::_fix();
    }
}
