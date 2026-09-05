#include "AGRemapCore/RemapService.h"

#include <filesystem>
#include <exception>
#include <stdexcept>
#include <typeinfo>
#include <system_error>
#include <unordered_map>
#include <utility>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/FilePrefixes.h"
#include "AGRemapCore/constants/FileSuffixes.h"
#include "AGRemapCore/constants/FileTypes.h"
#include "AGRemapCore/tools/Heading.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/files/FileService.h"


namespace AGRemapCore {
    RemapService::RemapService(std::optional<std::string> path, bool keepBackups, bool fixOnly, bool undoOnly,
                               bool hideOrig, bool readAllInis,
                               std::optional<std::unordered_set<int>> fromModTypeIds,
                               std::optional<std::unordered_set<int>> forcedModTypeIds,
                               tsl::ordered_set<int> defaultModTypeIds, bool handleExceptions,
                               std::optional<Version> fromVersion,
                               std::optional<std::unordered_set<int>> toModTypeIds,
                               std::optional<std::string> proxy, DownloadMode downloadMode,
                               std::optional<int> gameTypeId, std::shared_ptr<BaseLogger> logger):
        keepBackups(keepBackups),
        fixOnly(fixOnly),
        undoOnly(undoOnly),
        hideOrig(hideOrig),
        readAllInis(readAllInis),
        fromModTypeIds(std::move(fromModTypeIds)),
        forcedModTypeIds(std::move(forcedModTypeIds)),
        defaultModTypeIds(std::move(defaultModTypeIds)),
        handleExceptions(handleExceptions),
        fromVersion(std::move(fromVersion)),
        toModTypeIds(std::move(toModTypeIds)),
        proxy(std::move(proxy)),
        downloadMode(downloadMode),
        gameTypeId(gameTypeId),
        logger(std::move(logger)) {

        _setupModPath(std::move(path));
    }

    const std::string& RemapService::path() const {
        return path_;
    }

    void RemapService::setPath(std::optional<std::string> newPath) {
        _setupModPath(std::move(newPath));
        clear();
    }

    bool RemapService::pathIsCwd() const {
        return pathIsCwd_;
    }

    void RemapService::clear(bool clearLog) {
        stats.clear();

        if (clearLog && logger != nullptr) {
            logger->clear();
        }
    }

    void RemapService::FolderWalk::push(const std::string& folder) {
        if (visited.count(folder) > 0) {
            return;
        }

        // 'visiting' rather than a scan of 'dirs': the same folder can be named by many different
        // .ini files' resources, and the deque has no cheap membership test.
        if (visiting.insert(folder).second) {
            dirs.push_back(folder);
        }
    }

    void RemapService::fix() {
        // Restored once the walk finishes, so the caller's view is left with the prefix it came in
        // with rather than whichever folder happened to be visited last.
        std::optional<std::string> originalPrefix;
        if (logger != nullptr) {
            originalPrefix = logger->prefix();
        }

        try {
            _fix();
        } catch (const std::exception& exception) {
            if (logger != nullptr) {
                logger->setPrefix(*originalPrefix);
            }

            // Matching the pure-Python original: 'handleExceptions' is the caller saying it wants
            // the run to stop quietly rather than to see the exception. Writing the log file and
            // printing a summary afterwards is RemapServiceCLI's, not this class's.
            if (!handleExceptions) {
                throw;
            }

            if (logger != nullptr) {
                logger->handleException(exception);
            }

            return;
        }

        if (logger != nullptr) {
            logger->setPrefix(*originalPrefix);
        }

        report();
    }


    bool RemapService::noErrors() const {
        return stats.ini.skipped.empty() && stats.blend.skippedByMods.empty();
    }


    void RemapService::report() {
        if (logger == nullptr) {
            return;
        }

        logger->split();
        reportSkippedMods();
        logger->space();
        reportSummary();

        // The footer, on the same condition RemapServiceCLI gates its tips on -- see noErrors().
        if (noErrors()) {
            logger->space();
            logger->log("ENJOY");
        }

        logger->split();
    }


    namespace {
        // The exception's own message, and the closest thing C++ has to Python's type(e).__name__.
        // MSVC's typeid name is decorated ("class std::runtime_error"), which is close enough for a
        // warning a person reads and is the only portable handle on the type at all.
        std::pair<std::string, std::string> describeException(std::exception_ptr error) {
            if (!error) {
                return {"Error", ""};
            }

            try {
                std::rethrow_exception(error);
            } catch (const std::exception& exception) {
                return {typeid(exception).name(), exception.what()};
            } catch (...) {
                return {"Error", "an unknown error"};
            }
        }
    }


    std::string RemapService::_warnSkippedIniResource(const std::string& modPath, const FileStats& resourceStats) const {
        const std::string parentFolder = std::filesystem::path(path_).parent_path().string();
        Heading modHeading("Mod: " + FileService::getRelPath(modPath, parentFolder), 5);

        std::string message = modHeading.open() + "\n\n";

        auto modEntry = resourceStats.skippedByMods.find(modPath);
        if (modEntry != resourceStats.skippedByMods.end()) {
            for (const auto& fileEntry : modEntry->second) {
                auto [errorName, errorMessage] = describeException(fileEntry.second);
                Heading errorHeading(errorName, 3, "-");

                message += BaseLogger::getBulletStr(
                    FileService::getRelPath(fileEntry.first, path_) + ":\n\t"
                    + errorHeading.open() + "\n\t" + errorMessage + "\n\n");
            }
        }

        message += modHeading.close() + "\n";
        return message;
    }


    void RemapService::_reportSkippedAsset(const std::string& assetName,
                                            const std::unordered_map<std::string, std::unordered_map<std::string, std::exception_ptr>>& byMods,
                                            const FileStats& resourceStats) {
        if (byMods.empty()) {
            return;
        }

        std::string message = "\nWARNING: The following " + assetName
            + " were skipped due to warnings (see log above):\n\n";

        for (const auto& entry : byMods) {
            message += _warnSkippedIniResource(entry.first, resourceStats);
        }

        logger->error(message);
        logger->space();
    }


    void RemapService::reportSkippedMods() {
        if (logger == nullptr) {
            return;
        }

        _reportSkippedAsset("newly added " + FileTypes::Texture + " files", stats.texAdd.skippedByMods, stats.texAdd);
        _reportSkippedAsset("editted " + FileTypes::Texture + " files", stats.texEdit.skippedByMods, stats.texEdit);

        // The .ini files are the one bucket reported flat rather than grouped by mod folder -- a
        // skipped .ini file IS the thing that failed, so there is no inner list to group.
        if (!stats.ini.skipped.empty()) {
            std::string message = "\nWARNING: The following " + FileTypes::Ini
                + "s were skipped due to warnings (see log above):\n\n";

            for (const auto& entry : stats.ini.skipped) {
                auto [errorName, errorMessage] = describeException(entry.second);
                Heading errorHeading(errorName, 3, "-");

                message += BaseLogger::getBulletStr(entry.first + ":\n\t"
                    + errorHeading.open() + "\n\t" + errorMessage + "\n\n");
            }

            logger->error(message);
            logger->space();
        }

        _reportSkippedAsset(FileTypes::Blend + " files", stats.blend.skippedByMods, stats.blend);
        _reportSkippedAsset(FileTypes::Position + ", files", stats.position.skippedByMods, stats.position);
    }


    void RemapService::reportSummary() {
        if (logger == nullptr) {
            return;
        }

        const std::size_t fixedBlends = stats.blend.fixed.size();
        const std::size_t skippedBlends = stats.blend.skipped.size();
        const std::size_t removedRemapBlends = stats.blend.removed.size();
        const std::size_t foundBlends = fixedBlends + skippedBlends;

        const std::size_t fixedPositions = stats.position.fixed.size();
        const std::size_t skippedPositions = stats.position.skipped.size();
        const std::size_t removedRemapPositions = stats.position.removed.size();
        const std::size_t foundPositions = fixedPositions + skippedPositions;

        const std::size_t fixedInis = stats.ini.fixed.size();
        const std::size_t skippedInis = stats.ini.skipped.size();
        const std::size_t undoedInis = stats.ini.undoed.size();
        const std::size_t foundInis = fixedInis + skippedInis;

        const std::size_t fixedAddTextures = stats.texAdd.fixed.size();
        const std::size_t skippedAddTextures = stats.texAdd.skipped.size();
        const std::size_t removedTextures = stats.texAdd.removed.size();
        const std::size_t foundAddTextures = fixedAddTextures + skippedAddTextures;

        const std::size_t fixedEditTextures = stats.texEdit.fixed.size();
        const std::size_t skippedEditTextures = stats.texEdit.skipped.size();
        const std::size_t foundEditTextures = fixedEditTextures + skippedEditTextures;

        const std::size_t downloadedFiles = stats.download.fixed.size();
        const std::size_t cachedDownloadedFiles = stats.download.hit.size();
        const std::size_t skippedDownloads = stats.download.skipped.size();
        const std::size_t foundDownloads = downloadedFiles + cachedDownloadedFiles + skippedDownloads;
        const std::size_t removedDownloads = stats.download.removed.size();

        logger->openHeading("Summary", 10);
        logger->space();

        std::vector<std::string> lines;
        auto num = [](std::size_t value) { return std::to_string(value); };

        if (!undoOnly) {
            // These two unconditionally, matching the original: a run that found no .ini files and
            // no blends is exactly the outcome worth stating rather than staying silent about.
            lines.push_back("Out of the " + num(foundInis) + " " + FileTypes::Ini + "s within the found mods, fixed "
                            + num(fixedInis) + " " + FileTypes::Ini + "s and skipped " + num(skippedInis) + " " + FileTypes::Ini + "s");
            lines.push_back("Out of the " + num(foundBlends) + " " + FileTypes::Blend + " files within the found mods, fixed "
                            + num(fixedBlends) + " " + FileTypes::Blend + " files and skipped " + num(skippedBlends) + " " + FileTypes::Blend + " files");

            if (foundPositions > 0) {
                lines.push_back("Out of the " + num(foundPositions) + " " + FileTypes::Position + " files within the found mods, fixed "
                                + num(fixedPositions) + " " + FileTypes::Position + " files and skipped " + num(skippedPositions) + " " + FileTypes::Position + " files");
            }

            if (foundAddTextures > 0) {
                lines.push_back("Out of the " + num(foundAddTextures) + " " + FileTypes::Texture
                                + " files that were attempted to be created in the found mods, created "
                                + num(fixedAddTextures) + " " + FileTypes::Texture + " files and skipped "
                                + num(skippedAddTextures) + " " + FileTypes::Texture + " files");
            }

            if (foundEditTextures > 0) {
                lines.push_back("Out of the " + num(foundEditTextures) + " " + FileTypes::Texture + " files within the found mods, editted "
                                + num(fixedEditTextures) + " " + FileTypes::Texture + " files and skipped "
                                + num(skippedEditTextures) + " " + FileTypes::Texture + " files");
            }

            if (foundDownloads > 0) {
                lines.push_back("Out of " + num(foundDownloads) + " download requests within the found mods, downloaded "
                                + num(downloadedFiles) + " files, copied " + num(cachedDownloadedFiles)
                                + " files from existing downloads and skipped " + num(skippedDownloads) + " downloads");
            }
        }

        if (!fixOnly && undoedInis > 0) {
            std::string undoedMessage = "Removed fix from up to " + num(undoedInis) + " " + FileTypes::Ini + "s";
            if (undoOnly) {
                undoedMessage += " and skipped " + num(skippedInis) + " " + FileTypes::Ini + "s";
            }

            lines.push_back(undoedMessage);
        }

        if (!fixOnly && removedRemapBlends > 0) {
            lines.push_back("Removed " + num(removedRemapBlends) + " old " + FileTypes::RemapBlend + " files");
        }

        if (!fixOnly && removedRemapPositions > 0) {
            lines.push_back("Removed " + num(removedRemapPositions) + " old " + FileTypes::RemapPosition + " files");
        }

        if (!fixOnly && removedTextures > 0) {
            lines.push_back("Removed " + num(removedTextures) + " old " + FileTypes::RemapTexture + " files");
        }

        if (!fixOnly && removedDownloads > 0) {
            lines.push_back("Removed " + num(removedDownloads) + " old " + FileTypes::RemapDownload + " files");
        }

        for (const std::string& line : lines) {
            logger->bulletPoint(line);
        }

        logger->space();
        logger->closeHeading();
    }


    void RemapService::_fix() {
        // The walk reports each folder relative to the parent of where it started, so the starting
        // folder itself still shows up by name rather than as ".".
        const std::string parentFolder = std::filesystem::path(path_).parent_path().string();

        FolderWalk walk;
        walk.push(path_);

        while (!walk.dirs.empty()) {
            const std::string folder = std::move(walk.dirs.back());
            walk.dirs.pop_back();
            walk.visiting.erase(folder);

            if (walk.visited.count(folder) > 0) {
                continue;
            }

            walk.visited.insert(folder);

            if (logger != nullptr) {
                logger->split();
                logger->setPrefix(FileService::getRelPath(folder, parentFolder));
            }

            // Before anything in the folder is handled, and only for a folder being seen for the
            // first time. The pure-Python original does this per MOD (fixMod's own
            // "if (not self.keepBackups): mod.removeBackupInis()"), which is the same moment: the
            // first time that folder is reached.
            if (!keepBackups) {
                _removeBackupInis(folder);
            }

            const std::vector<std::string> iniPaths = _getIniPaths(folder);
            const std::size_t iniPathsLen = iniPaths.size();

            for (std::size_t iniInd = 0; iniInd < iniPathsLen; ++iniInd) {
                const std::string& iniPath = iniPaths[iniInd];
                std::unique_ptr<IniFile> ini = createIni(iniPath);

                // One bad .ini file stops that .ini file, not the whole walk -- the pure-Python
                // original guarded 'createMod'/'fixMod' the same way.
                try {
                    handleIni(*ini);
                } catch (const std::exception& exception) {
                    if (logger != nullptr) {
                        logger->handleException(exception);
                    }

                    // Recorded here rather than in handleIni for the same reason the original
                    // records it in fixMod rather than fixIni: handleIni is the thing that failed,
                    // so it cannot be the thing that files the report. No mod folder is attributed
                    // (unlike a resource's), matching stats.ini.addSkipped's own call in fixMod --
                    // which is why noErrors() checks stats.ini.skipped directly and not
                    // skippedByMods.
                    stats.ini.addSkipped(iniPath, std::current_exception());
                }

                // Deliberately outside the try: a .ini file that failed part way through may still
                // have parsed enough resources to name folders worth visiting.
                addIniNeighbourFolders(walk, *ini);

                // A blank line between .ini files, but not after the last one. Undoing is quiet
                // enough not to need the separation.
                if (!undoOnly && iniInd + 1 < iniPathsLen && logger != nullptr) {
                    logger->space();
                }
            }

            // Runs whether or not the folder held any .ini files. A mod's own subfolders are not
            // all reachable through its resources -- an .ini file references the resources it
            // knows about, and a folder can hold another mod entirely.
            addNeighbourFolders(walk, folder);
        }
    }

    bool RemapService::_isBackupIni(const std::string& file) {
        const std::string baseName = std::filesystem::path(file).filename().string();

        // Either extension: the backup this fix writes is a .txt (disableIni changes the extension
        // so a mod loader stops reading it), while the ones older versions left behind kept .ini.
        if (!StringTools::endsWithIgnoreCase(baseName, FileExt::Ini)
                && !StringTools::endsWithIgnoreCase(baseName, FileExt::Txt)) {
            return false;
        }

        static const std::vector<const std::string*> backupMarkers = {
            &FilePrefixes::BackupFilePrefix,
            &FilePrefixes::OldBackupFilePrefixV3,
            &FilePrefixes::OldBackupFilePrefixV4_3
        };

        for (const std::string* marker : backupMarkers) {
            if (baseName.find(*marker) != std::string::npos) {
                return true;
            }
        }

        return false;
    }


    void RemapService::_removeBackupInis(const std::string& folder) {
        // Direct children only. A backup belongs beside the .ini file it was made from, and the
        // walk reaches every folder in its own turn anyway -- recursing here would delete backups
        // in folders this run has not decided to visit yet.
        const std::vector<std::string> files = FileService::getFilesAndDirs(folder).first;

        for (const std::string& file : files) {
            if (!_isBackupIni(file)) {
                continue;
            }

            log("Removing the backup ini, " + std::filesystem::path(file).filename().string());

            // A backup that is already gone is not a failure -- the pure-Python original swallows
            // exactly this as a FileNotFoundError.
            std::error_code err;
            std::filesystem::remove(file, err);
        }
    }


    bool RemapService::_isSrcIni(const std::string& file) {
        const std::string baseName = std::filesystem::path(file).filename().string();

        // Case-insensitively: a mod folder in the wild is as likely to hold a ".INI" as a ".ini",
        // and Windows itself does not distinguish them.
        if (!StringTools::endsWithIgnoreCase(baseName, FileExt::Ini)) {
            return false;
        }

        // Anywhere in the base name, not just at its start/end -- the pure-Python original
        // (Mod::getOptionalFiles) matches all four of these as bare substrings, and a backup is
        // named "<original name>RemapBKUP.txt"-style rather than strictly prefixed.
        static const std::vector<const std::string*> ownMarkers = {
            &FilePrefixes::BackupFilePrefix,
            &FilePrefixes::OldBackupFilePrefixV3,
            &FilePrefixes::OldBackupFilePrefixV4_3,
            &FileSuffixes::RemapFixCopy
        };

        for (const std::string* marker : ownMarkers) {
            if (baseName.find(*marker) != std::string::npos) {
                return false;
            }
        }

        return true;
    }

    bool RemapService::_isRemapCopyIni(const std::string& file) {
        const std::string baseName = std::filesystem::path(file).filename().string();

        return StringTools::endsWithIgnoreCase(baseName, FileExt::Ini)
            && baseName.find(FileSuffixes::RemapFixCopy) != std::string::npos;
    }


    std::string RemapService::_origIniPath(const std::string& remapCopyPath) {
        const std::filesystem::path path(remapCopyPath);
        const std::string baseName = path.filename().string();

        // Split on the LAST occurrence: a mod's own .ini file is free to have "RemapFix" in its
        // name, and it is the suffix this fix appended that has to come off, not the first match.
        const std::size_t suffixPos = baseName.rfind(FileSuffixes::RemapFixCopy);
        if (suffixPos == std::string::npos) {
            return remapCopyPath;
        }

        const std::string head = baseName.substr(0, suffixPos);
        std::string tail = baseName.substr(suffixPos + FileSuffixes::RemapFixCopy.size());

        // What follows the suffix is the copy's index and then the extension ("1.ini"). Everything
        // up to the first '.' is that index and goes; the extension stays. A tail starting AT the
        // '.' has no index to drop, which is why the position must be greater than zero rather than
        // merely found -- exactly the check Mod::getOrigIniPath makes.
        const std::size_t extPos = tail.find('.');
        if (extPos > 0 && extPos != std::string::npos) {
            tail = tail.substr(extPos);
        }

        return (path.parent_path() / (head + tail)).string();
    }


    void RemapService::_removeRemapCopies(const IniFile& ini) {
        if (!ini.getFile().has_value()) {
            return;
        }

        const std::string iniPath = *ini.getFile();
        const std::string folder = std::filesystem::path(iniPath).parent_path().string();

        // Found by walking the folder and mapping each copy back to its source, rather than by
        // guessing how many copies exist: that is what the pure-Python original's
        // 'groupedRemapCopies' is, just built on demand instead of up front.
        const std::vector<std::string> files = FileService::getFilesAndDirs(folder).first;
        bool removedAny = false;

        for (const std::string& file : files) {
            if (!_isRemapCopyIni(file) || _origIniPath(file) != iniPath) {
                continue;
            }

            // Undone before it is deleted, not merely deleted. A copy's fix owns resources of its
            // own (the whole reason it exists is to carry a mod object the source file could not),
            // and deleting the file would strand them -- so the removal runs first and reports
            // them, and only then does the file go.
            //
            // keepBackups is false whatever this run asked for: a backup of a file this fix
            // generated protects nothing.
            std::unique_ptr<IniFile> copy = createIni(file);
            copy->classify();
            copy->removeFix(false, true, readAllInis, false);

            std::error_code err;
            if (!std::filesystem::remove(file, err) || err) {
                continue;
            }

            log("Removing the .ini remap copy, " + std::filesystem::path(file).filename().string());
            removedAny = true;
        }

        if (removedAny && logger != nullptr) {
            logger->space();
        }
    }


    std::vector<std::string> RemapService::_getIniPaths(const std::string& folder) {
        std::vector<std::string> result;

        const std::vector<std::string> files = FileService::getFilesAndDirs(folder).first;
        for (const std::string& file : files) {
            if (_isSrcIni(file)) {
                result.push_back(file);
            }
        }

        return result;
    }

    void RemapService::handleIni(IniFile& ini) {
        ini.classify();

        // Not a mod's .ini file at all, or one nothing recognised while the caller only asked for
        // recognised ones. Either way there is nothing here this fix has any business touching.
        if (!ini.getIsMod() || (ini.getModTypes().empty() && !readAllInis)) {
            return;
        }

        const std::string iniName = _iniName(ini);

        // Empty for a .ini file built from raw text rather than a path. Every stats bucket below is
        // keyed by path, so those simply go untracked rather than all colliding under "".
        const std::string iniPath = ini.getFile().value_or("");

        if (!fixOnly) {
            // Read BEFORE the removal, because the removal is what makes it false. classify() has
            // already run, so this is the classifier's answer about the file as it was found.
            const bool wasFixed = ini.getIsFixed();
            log("Removing the fix from " + iniName + "...");

            // The removal reports what it took per kind of resource, and every one of those kind
            // names is a RemapStats member name -- see RemapStats::get. That is what lets this sort
            // them without a lookup table of its own.
            std::unordered_map<std::string, std::vector<std::unique_ptr<IniResource>>> removedResources;
            ini.removeFix(false, !undoOnly, readAllInis, keepBackups, &removedResources);

            bool removedAny = false;
            for (auto& entry : removedResources) {
                FileStats* resourceStats = stats.get(entry.first);

                for (std::unique_ptr<IniResource>& resource : entry.second) {
                    if (resource == nullptr) {
                        continue;
                    }

                    if (!removedAny) {
                        log("Removing the fixed resources from " + iniName + "...");
                    }

                    std::error_code removeError;
                    std::filesystem::remove(resource->srcPath, removeError);

                    // A file already gone is a removal that has nothing left to do, not a failure --
                    // the pure-Python original swallows exactly this as a FileNotFoundError.
                    removedAny = true;

                    // An unknown kind is tracked nowhere rather than being forced into a bucket it
                    // does not belong in. The file is still deleted -- only the bookkeeping is lost.
                    if (resourceStats != nullptr) {
                        resourceStats->addRemoved(resource->srcPath);
                    }
                }
            }

            // "up to", as the summary itself hedges: what is counted is that a fixed .ini file went
            // through the removal, not that every trace of the fix provably came out.
            if (wasFixed && !iniPath.empty()) {
                stats.ini.addUndoed(iniPath);
            }

            // Gated on the file actually having been fixed, matching the original: a .ini file that
            // was not fixed has no copies of this fix's making, and anything named like one beside
            // it is not this run's to delete.
            if (wasFixed) {
                _removeRemapCopies(ini);
            }
        }

        if (undoOnly) {
            return;
        }

        // Three ways this .ini file needs no fixing, all of them from the pure-Python original's
        // own fixIni. The first two are keyed by PATH rather than by object: the walk can reach the
        // same file twice (a resource in one mod naming a folder another mod also lives in), and
        // the second visit builds a different IniFile for the same path.
        if (!iniPath.empty() && stats.ini.skipped.count(iniPath) > 0) {
            log("the ini file, " + iniName + ", has already encountered an error");
            return;
        }

        if (!iniPath.empty() && stats.ini.fixed.count(iniPath) > 0) {
            log("the ini file, " + iniName + ", is already fixed");
            return;
        }

        // The third is the file's own content saying so -- a fix that is already in there. Counted
        // as fixed, because it is: the desired end state holds.
        if (ini.getIsFixed()) {
            log("the ini file, " + iniName + ", is already fixed");

            if (!iniPath.empty()) {
                stats.ini.addFixed(iniPath);
            }

            return;
        }

        // The removal built resource models to work out what to delete; the fix must not inherit
        // them. Dropped rather than reused because the fix builds its own from a fresh parse, and a
        // model left over from the undo describes the file as it WAS.
        //
        // Placed after the early-outs rather than before, which is a small departure from the
        // pure-Python original (it clears every .ini file's models up front, before its fix loop).
        // The difference only shows on a path that returns early: those keep the removal's models,
        // and _fix reads them straight afterwards through addIniNeighbourFolders. Clearing first
        // would cost that file its folder discovery for nothing.
        ini.clearModels();

        ini.fix(keepBackups, fixOnly, hideOrig);
        fixResources(ini);

        // Last, and only on the way out: a fix that threw leaves this unreached, and _fix's own
        // catch records the file as skipped instead. The two are mutually exclusive by construction
        // rather than by a flag.
        if (!iniPath.empty()) {
            stats.ini.addFixed(iniPath);
        }
    }


    void RemapService::fixResources(IniFile& ini) {
        // Screened first, fixed second, rather than both in one pass. The heading below has to know
        // whether ANYTHING survived screening before the first resource is touched -- announcing
        // "fixing the resources" and then silently fixing none of them is worse than saying nothing.
        std::vector<IniResource*> toFix;

        for (std::unique_ptr<IniResource>& resource : ini.getResources()) {
            if (resource == nullptr) {
                continue;
            }

            // Not every resource is a remap resource -- one that is not carries none of the
            // questions below and no fix() this class knows how to call.
            RemapIniResourceMixin* remap = dynamic_cast<RemapIniResourceMixin*>(resource.get());
            if (remap == nullptr) {
                continue;
            }

            if (!remap->hasRequired()) {
                // Recorded, not silently dropped: the pure-Python original raises a
                // RemapMissingBlendFile here and files it under skipped, and a resource that cannot
                // be fixed is exactly the thing a summary needs to be able to report.
                FileStats* resourceStats = stats.get(resource->type);
                if (resourceStats != nullptr) {
                    resourceStats->addSkipped(resource->srcPath,
                                              std::make_exception_ptr(std::runtime_error(
                                                  "Missing the file(s) required to fix " + resource->srcPath)));
                }
                continue;
            }

            // Already dealt with, successfully or not -- on this .ini file or an earlier one, since
            // several .ini files can name the same source or fixed file.
            if (remap->srcIsFixed(stats) || remap->srcEncounteredError(stats)
                    || remap->fixIsFixed(stats) || remap->fixEncounteredError(stats)) {
                continue;
            }

            // "Fix without removing previous fixes" -- a fixed file already on disk is left alone.
            if (fixOnly && remap->fixExists(stats)) {
                continue;
            }

            toFix.push_back(resource.get());
        }

        if (toFix.empty()) {
            return;
        }

        log("Fixing the resources for " + _iniName(ini) + "...");

        for (IniResource* resource : toFix) {
            FileStats* resourceStats = stats.get(resource->type);

            try {
                if (!_fixResource(*resource)) {
                    continue;
                }
            } catch (const std::exception& exception) {
                if (resourceStats != nullptr) {
                    resourceStats->addSkipped(resource->srcPath, std::current_exception(), path_);
                }

                if (logger != nullptr) {
                    logger->handleException(exception);
                }

                continue;
            }

            if (resourceStats != nullptr) {
                resourceStats->addFixed(_fixedPathOf(*resource));
            }
        }
    }


    bool RemapService::_fixResource(IniResource& resource) {
        // There is deliberately no virtual fix() to call here: IniResource's own doc comment
        // records why -- every leaf needs a differently-typed one (a download takes the download
        // stats and the proxy; a blend/texture takes nothing), so a generic signature would fit
        // none of them. Dispatching on the concrete type is the cost of that, and it is paid in
        // exactly this one place.
        if (RemapIniDownload* download = dynamic_cast<RemapIniDownload*>(&resource)) {
            // Through remapFix rather than fix: it is the one that separates a real download from a
            // cache hit, and stats.download tracks those separately.
            return download->remapFix(stats, proxy);
        }

        if (RemapBlendResource* blend = dynamic_cast<RemapBlendResource*>(&resource)) {
            return blend->fix();
        }

        if (RemapTexAddResource* texAdd = dynamic_cast<RemapTexAddResource*>(&resource)) {
            return texAdd->fix();
        }

        if (RemapTexEditResource* texEdit = dynamic_cast<RemapTexEditResource*>(&resource)) {
            return texEdit->fix();
        }

        if (IniGroupedResource* grouped = dynamic_cast<IniGroupedResource*>(&resource)) {
            return grouped->fix();
        }

        return false;
    }


    std::string RemapService::_fixedPathOf(const IniResource& resource) {
        // What actually got written: a fix resource writes to its fixedPath, everything else works
        // in place at its srcPath.
        const IniFixResource* fixResource = dynamic_cast<const IniFixResource*>(&resource);
        if (fixResource != nullptr) {
            return fixResource->fixedPath;
        }

        return resource.srcPath;
    }


    std::string RemapService::_iniName(const IniFile& ini) {
        // Base name rather than the whole path: the logger already carries the folder as its prefix
        // (see _fix), so repeating it in every line would be noise.
        if (!ini.getFile().has_value()) {
            return "the .ini file";
        }

        return std::filesystem::path(*ini.getFile()).filename().string();
    }


    void RemapService::log(const std::string& message) {
        if (logger == nullptr) {
            return;
        }

        logger->log(message);
    }

    void RemapService::addNeighbourFolders(FolderWalk& walk, const std::string& folder) {
        if (walk.gotNeighbours.count(folder) > 0) {
            return;
        }

        const std::vector<std::string> dirs = FileService::getFilesAndDirs(folder, true).second;

        for (const std::string& dir : dirs) {
            // Every one of these came out of the *recursive* scan of 'folder', so each of them has
            // now had its own subtree enumerated too -- visiting any of them later must not walk
            // the same subtree a second time.
            walk.gotNeighbours.insert(dir);
            walk.push(dir);
        }

        walk.gotNeighbours.insert(folder);
    }

    void RemapService::addIniNeighbourFolders(FolderWalk& walk, const IniFile& ini) {
        for (const std::string& folder : ini.getReferencedFolders()) {
            walk.push(folder);
        }
    }

    std::unique_ptr<IniFile> RemapService::createIni(const std::string& iniPath) {
        // Straight pass-through: these three are stored in IniFile's own convention (std::nullopt
        // = no filter, present-but-empty = accept nothing) precisely so nothing needs
        // reinterpreting here.
        std::unique_ptr<IniFile> result = std::make_unique<IniFile>(iniPath, "", gameTypeId,
                                                                   fromModTypeIds, forcedModTypeIds,
                                                                   std::nullopt, nullptr, std::nullopt,
                                                                   downloadMode, fromVersion, std::nullopt,
                                                                   toModTypeIds);

        // Assigned, not passed: public members on IniFile with no constructor parameter, exactly
        // like its 'fromVersion'.
        result->defaultModTypeIds = defaultModTypeIds;

        // The same view this service reports through, so a .ini file's own messages land beside the
        // walk's rather than disappearing.
        result->logger = logger;

        return result;
    }

    void RemapService::_setupModPath(std::optional<std::string> newPath) {
        pathIsCwd_ = false;

        if (!newPath.has_value()) {
            path_ = FileService::defaultPath();
            pathIsCwd_ = true;
            return;
        }

        path_ = FileService::parseOSPath(std::filesystem::absolute(std::filesystem::path(*newPath)).string());
        pathIsCwd_ = (path_ == FileService::defaultPath());
    }
}
