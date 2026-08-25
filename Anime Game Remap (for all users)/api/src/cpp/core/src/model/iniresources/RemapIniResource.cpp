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
            std::filesystem::rename(from, to, renameError);
            if (!renameError) {
                return;
            }

            std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
            std::filesystem::remove(from);
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
        return std::filesystem::exists(fixedPath);
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
        std::string downloadFolder = std::filesystem::path(srcPath).parent_path().string();
        auto [rawDownloadFullPath, downloaded, downloadExisted] = download->get(downloadFolder, proxy);
        (void)downloadExisted;

        if (srcPath != rawDownloadFullPath) {
            moveFile(rawDownloadFullPath, srcPath);
        }

        if (downloaded) {
            downloadStats.addFixed(srcPath);
        } else {
            downloadStats.addHit(srcPath);
        }

        return downloaded;
    }

    bool RemapIniDownload::fix(CachedFileStats& downloadStats, std::optional<std::string> proxy) {
        if (fixFunc) {
            return fixFunc(*this, downloadStats);
        }

        return _fix(downloadStats, proxy);
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
