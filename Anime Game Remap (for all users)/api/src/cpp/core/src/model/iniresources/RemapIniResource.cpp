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
#include "AGRemapCore/model/iniresources/RemapIniResource.h"

#include <filesystem>
#include <system_error>
#include <utility>


namespace AGRemapCore {

    namespace {
        // Mirrors Python's shutil.move -- a plain rename first (cheap, the common case), falling
        // back to copy-then-remove if that fails (eg. the source/destination are on different
        // filesystems/drives, which std::filesystem::rename doesn't handle the way shutil.move does).
        void moveFile(const std::string& from, const std::string& to) {
            std::error_code renameError;
            std::filesystem::rename(FileService::strToPath(from), to, renameError);
            if (!renameError) {
                return;
            }

            std::filesystem::copy_file(FileService::strToPath(from), to, std::filesystem::copy_options::overwrite_existing);
            std::filesystem::remove(FileService::strToPath(from));
        }
    }

    bool RemapIniResourceMixin::srcEncounteredError(const RemapStats& stats) const {
        (void)stats;
        return false;
    }

    bool RemapIniResourceMixin::srcIsFixed(const RemapStats& stats) const {
        (void)stats;
        return false;
    }

    bool RemapIniResourceMixin::fixEncounteredError(const RemapStats& stats) const {
        (void)stats;
        return false;
    }

    bool RemapIniResourceMixin::fixIsFixed(const RemapStats& stats) const {
        (void)stats;
        return false;
    }

    bool RemapIniResourceMixin::fixExists(const RemapStats& stats) const {
        (void)stats;
        return false;
    }

    bool RemapIniResourceMixin::hasRequired() const {
        return false;
    }

    bool RemapIniResource::hasRequired() const {
        return true;
    }

    bool RemapIniResource::fixExists(const RemapStats& stats) const {
        return srcIsFixed(stats);
    }

    bool RemapIniFixResource::hasRequired() const {
        return true;
    }

    bool RemapIniFixResource::fixExists(const RemapStats& stats) const {
        (void)stats;
        return std::filesystem::exists(FileService::strToPath(fixedPath));
    }

    RemapIniDownload::RemapIniDownload(const std::string& iniFolderPath, const std::string& srcPath, std::unique_ptr<FileDownload> download,
                                        std::string type, std::function<bool(RemapIniDownload&, CachedFileStats&)> fixFunc):
        RemapIniResource(std::move(type), iniFolderPath, srcPath), download(std::move(download)), fixFunc(std::move(fixFunc)) {}

    bool RemapIniDownload::srcEncounteredError(const RemapStats& stats) const {
        return stats.download.skipped.contains(srcPath);
    }

    bool RemapIniDownload::srcIsFixed(const RemapStats& stats) const {
        return stats.download.fixed.contains(srcPath);
    }

    bool RemapIniDownload::fixEncounteredError(const RemapStats& stats) const {
        return srcEncounteredError(stats);
    }

    bool RemapIniDownload::fixIsFixed(const RemapStats& stats) const {
        return srcIsFixed(stats);
    }

    bool RemapIniDownload::fixExists(const RemapStats& stats) const {
        return srcIsFixed(stats);
    }

    bool RemapIniDownload::_fix(CachedFileStats& downloadStats, std::optional<std::string> proxy) {
        std::string downloadFolder = FileService::pathToStr(FileService::strToPath(srcPath).parent_path());
        auto [rawDownloadFullPath, downloaded, downloadExisted] = download->get(downloadFolder, proxy, downloadCache);
        (void)downloadExisted;

        if (srcPath != rawDownloadFullPath) {
            moveFile(rawDownloadFullPath, srcPath);

            // The run's cache was told where get() PUT the file, and it is no longer there.
            // Correct it, or every later use of this URL pays a failed copy and re-downloads --
            // a cache that costs something and returns nothing.
            if (downloaded && downloadCache != nullptr && download != nullptr) {
                downloadCache->remember(download->url, srcPath);
            }
        }

        if (downloaded) {
            downloadStats.addFixed(srcPath);
        } else {
            downloadStats.addHit(srcPath);
        }

        return downloaded;
    }

    bool RemapIniDownload::willDownload() const {
        if (download == nullptr || fixFunc) {
            return true;
        }

        return !download->cachedPath(downloadCache).has_value();
    }

    bool RemapIniDownload::fix(CachedFileStats& downloadStats, std::optional<std::string> proxy) {
        // BEFORE the work, for the reason RemapBlendResource::fix gives and then some: a download
        // is the slowest thing this program does and much the likeliest to fail, so this is the
        // line that explains a long pause and the one still on screen when the request throws
        // ("Could not resolve hostname" being the usual one).
        //
        // A cache hit gets its own wording rather than the same one: copying a file this run has
        // already fetched is instant and cannot fail the way a request can, so calling it
        // "Downloading" would make a 36-.ini mod look like 36 trips to github when 35 of them
        // never leave the disk. No trailing "..." on that one for the same reason -- there is
        // nothing to wait for.
        const std::string name = FileService::pathToStr(FileService::strToPath(srcPath).filename());
        const bool expectingDownload = willDownload();

        if (logger != nullptr) {
            logger->log(expectingDownload ? "Downloading " + name + "..."
                                          : "Copying download " + name);
        }

        const bool downloaded = fixFunc ? fixFunc(*this, downloadStats) : _fix(downloadStats, proxy);

        // The prediction was that a copy would do, and it did not -- FileDownload::get found the
        // remembered file gone and went to the network after all. Rare, but saying so is the
        // difference between a log that describes the run and one that describes an intention.
        if (!expectingDownload && downloaded && logger != nullptr) {
            logger->log("Downloading " + name + "...");
        }

        return downloaded;
    }

    bool RemapIniDownload::remapFix(RemapStats& stats, std::optional<std::string> proxy,
                                     const std::function<void(const std::string&)>& downloadHandler,
                                     const std::function<void(const std::string&)>& cacheHitHandler) {
        bool downloaded = fix(stats.download, proxy);

        if (downloaded) {
            if (downloadHandler) {
                downloadHandler(srcPath);
            }
        } else {
            if (cacheHitHandler) {
                cacheHitHandler(srcPath);
            }
        }

        return downloaded;
    }
}
