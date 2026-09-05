#include "AGRemapCore/RemapServiceCLI.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <utility>
#include <vector>

#include "AGRemapCore/RemapServiceCLIErrors.h"
#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/constants/FileTypes.h"
#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/Version.h"


namespace AGRemapCore {
    RemapServiceCLI::RemapServiceCLI(RemapService service, std::optional<std::string> log, bool verbose):
        service(std::move(service)),
        // logTxt is driven by whether a log was asked for: with no log folder there is nothing to
        // accumulate for, so BaseLogger keeps nothing rather than buffering text no one will read.
        logger(std::make_shared<Logger>("", log.has_value(), verbose)) {

        _setupLogPath(std::move(log));

        // Overwritten rather than merged: reporting is what this class exists for, so whatever the
        // model was handed before is replaced by the view that owns the log file.
        this->service.logger = logger;
    }

    RemapServiceCLI::RemapServiceCLI(std::optional<std::string> path, bool keepBackups, bool fixOnly,
                                     bool undoOnly, bool hideOrig, bool readAllInis,
                                     std::optional<std::vector<std::string>> types,
                                     std::optional<std::string> defaultType,
                                     std::optional<std::string> forcedType,
                                     std::optional<std::string> log, bool verbose,
                                     bool handleExceptions, std::optional<std::string> version,
                                     std::optional<std::vector<std::string>> remappedTypes,
                                     std::optional<std::string> proxy,
                                     std::optional<std::string> downloadMode,
                                     std::optional<int> gameTypeId):
        // Everything already unambiguous goes straight in. The seven string-shaped options are left
        // at their defaults here and filled in by the setups below, which is where they can fail.
        service(std::move(path), keepBackups, fixOnly, undoOnly, hideOrig, readAllInis),
        logger(std::make_shared<Logger>("", log.has_value(), verbose)) {

        _setupLogPath(std::move(log));
        service.logger = logger;
        service.handleExceptions = handleExceptions;
        service.proxy = std::move(proxy);
        service.gameTypeId = gameTypeId;

        // Nothing resolves a mod type by name until the shipped ones are filed, and on a normal run
        // nothing has filed them yet -- the registry is otherwise populated as a side effect of the
        // first classify(), which happens long after this. registerMissing rather than registerAll
        // so a caller that registered its own mod type under a shipped id keeps it.
        GlobalModTypes::registerMissing();

        // The pure-Python original's own order, and it matters: the two after the forced type read
        // whether one was given, and the fix-from types can be decided entirely by it.
        _setupForcedModType(forcedType);
        _setupDefaultModType(defaultType, forcedType);
        _setupToFixModTypes(types, forcedType);
        _setupRemappedTypes(remappedTypes);
        _setupVersion(version);
        _setupDownloadMode(downloadMode);
    }

    bool RemapServiceCLI::hasErrorsBeforeFix() const {
        return errorsBeforeFix_ != nullptr;
    }

    void RemapServiceCLI::raiseErrorsBeforeFix() const {
        if (errorsBeforeFix_ != nullptr) {
            std::rethrow_exception(errorsBeforeFix_);
        }
    }

    void RemapServiceCLI::_recordError(std::exception_ptr error) {
        // First one wins. Every setup still runs afterwards -- they are independent, and stopping
        // early would only mean a second, unrelated mistake goes unreported the next time round --
        // but what surfaces is the first thing that was wrong.
        if (errorsBeforeFix_ == nullptr) {
            errorsBeforeFix_ = std::move(error);
        }
    }

    std::optional<int> RemapServiceCLI::_toModTypeId(const std::string& name) {
        const std::optional<ModTypeId> found = ModTypeIdTools::findByName(name);

        if (!found.has_value()) {
            _recordError(std::make_exception_ptr(InvalidModType(name)));
            return std::nullopt;
        }

        return static_cast<int>(*found);
    }

    std::optional<std::unordered_set<int>> RemapServiceCLI::_toModTypeIds(const std::optional<std::vector<std::string>>& names) {
        // Absent and empty are the SAME answer here: naming no mod types is asking for all of them,
        // which is what an argument parser produces when the option is left off. RemapService's own
        // empty set means the opposite (a filter that accepts nothing), and this is the seam where
        // that ambiguity gets resolved -- so this hands back nullopt for both.
        if (!names.has_value() || names->empty()) {
            return std::nullopt;
        }

        std::unordered_set<int> result;

        for (const std::string& name : *names) {
            const std::optional<int> modTypeId = _toModTypeId(name);

            // Keep going rather than bailing on the first bad name. The error is already recorded
            // and the run will raise it; collecting the rest costs nothing and leaves the object in
            // a state worth inspecting.
            if (modTypeId.has_value()) {
                result.insert(*modTypeId);
            }
        }

        return result;
    }

    void RemapServiceCLI::_setupForcedModType(const std::optional<std::string>& forcedType) {
        if (!forcedType.has_value()) {
            return;
        }

        const std::optional<int> modTypeId = _toModTypeId(*forcedType);
        if (modTypeId.has_value()) {
            service.forcedModTypeIds = std::unordered_set<int>{*modTypeId};
        }
    }

    void RemapServiceCLI::_setupDefaultModType(const std::optional<std::string>& defaultType,
                                               const std::optional<std::string>& forcedType) {
        service.defaultModTypeIds.clear();

        // Nothing to fall back FOR unless unclassified .ini files are being read, and nothing to
        // fall back TO when a forced type already answers the question for every file. Note this
        // asks whether a forced type was GIVEN, not whether it resolved -- a run whose forced type
        // was a typo is going to raise, so what the default would have been does not matter.
        if (!service.readAllInis || forcedType.has_value()) {
            return;
        }

        if (!defaultType.has_value()) {
            service.defaultModTypeIds.insert(static_cast<int>(ModTypeId::Raiden));
            return;
        }

        const std::optional<int> modTypeId = _toModTypeId(*defaultType);
        if (modTypeId.has_value()) {
            service.defaultModTypeIds.insert(*modTypeId);
        }
    }

    void RemapServiceCLI::_setupToFixModTypes(const std::optional<std::vector<std::string>>& types,
                                              const std::optional<std::string>& forcedType) {
        // A forced type IS the answer: every .ini file is treated as that type, so there is nothing
        // for a fix-from filter to narrow and whatever was named alongside it is ignored.
        if (forcedType.has_value()) {
            if (service.forcedModTypeIds.has_value()) {
                service.fromModTypeIds = *service.forcedModTypeIds;
            }
            return;
        }

        // Reading every .ini file means fixing every type, whatever was named.
        if (service.readAllInis) {
            service.fromModTypeIds = std::nullopt;
            return;
        }

        service.fromModTypeIds = _toModTypeIds(types);
    }

    void RemapServiceCLI::_setupRemappedTypes(const std::optional<std::vector<std::string>>& remappedTypes) {
        service.toModTypeIds = _toModTypeIds(remappedTypes);
    }

    void RemapServiceCLI::_setupVersion(const std::optional<std::string>& version) {
        if (!version.has_value()) {
            return;
        }

        const std::optional<Version> parsed = Version::parse(*version);

        if (!parsed.has_value()) {
            _recordError(std::make_exception_ptr(InvalidVersion(*version)));
            return;
        }

        service.fromVersion = parsed;
    }

    void RemapServiceCLI::_setupDownloadMode(const std::optional<std::string>& downloadMode) {
        // Unset is Normal. The pure-Python original named a 'HardTexDriven' mode here that has since
        // been removed from the enum, so its own unset case raises an AttributeError -- there is no
        // behaviour left to be faithful to, and Normal is the maintainer's choice of replacement.
        if (!downloadMode.has_value()) {
            service.downloadMode = DownloadMode::Normal;
            return;
        }

        const std::optional<DownloadMode> found = DownloadModeTools::findByName(*downloadMode);

        if (!found.has_value()) {
            _recordError(std::make_exception_ptr(InvalidDownloadMode(*downloadMode)));
            return;
        }

        service.downloadMode = *found;
    }

    const std::optional<std::string>& RemapServiceCLI::getLog() const {
        return log_;
    }

    void RemapServiceCLI::setLog(std::optional<std::string> newLog) {
        _setupLogPath(std::move(newLog));

        if (logger != nullptr) {
            logger->logTxt = log_.has_value();
        }
    }

    bool RemapServiceCLI::getVerbose() const {
        // No view means nothing is printed, whatever was asked for -- which is the honest answer
        // here rather than reporting back the flag a caller last set.
        return logger != nullptr && logger->verbose;
    }

    void RemapServiceCLI::setVerbose(bool newVerbose) {
        if (logger != nullptr) {
            logger->verbose = newVerbose;
        }
    }

    void RemapServiceCLI::fix() {
        // Before everything, including the banner: a run whose options could not be understood is
        // not going to happen, and a banner listing the mod types that DID resolve would be a
        // half-truth. The pure-Python original skipped it for the same reason, by not printing it
        // from a constructor that had already failed.
        //
        // 'handleExceptions' is honoured here rather than deferred to the model, because the model
        // never sees this error -- it is the CLI's own, from text the model does not take. Same two
        // outcomes though: logged and swallowed, or thrown.
        if (hasErrorsBeforeFix()) {
            try {
                raiseErrorsBeforeFix();
            } catch (const std::exception& exception) {
                if (!service.handleExceptions) {
                    createLog();
                    throw;
                }

                if (logger != nullptr) {
                    logger->handleException(exception);
                }
            }

            createLog();
            return;
        }

        // Here rather than in the constructor, where the pure-Python original put it: a virtual call
        // from a constructor does not dispatch to an override, so a subclass reshaping the banner
        // would silently never be reached. The driver constructs and then fixes, so what the user
        // sees is unchanged.
        printModsToFix();

        // The log is written whatever happened. A run that threw is exactly the one whose log is
        // worth keeping, so this is not in an "on success" branch -- and the exception still leaves
        // by its own route once the file is on disk.
        try {
            service.fix();
        } catch (...) {
            createLog();
            throw;
        }

        // Before the log rather than after, so the tips are IN the log file -- and only after a
        // clean run, since advice about what to try next reads badly under a list of failures.
        if (service.noErrors()) {
            addTips();
        }

        createLog();
    }

    void RemapServiceCLI::printModsToFix() {
        if (logger == nullptr) {
            return;
        }

        // A heading is already visually marked out; the per-line prefix on top of it is noise. Saved
        // and restored rather than forced back to true, so this cannot quietly turn on a prefix a
        // caller had deliberately turned off.
        const bool hadPrefix = logger->includePrefix;
        logger->includePrefix = false;

        logger->openHeading("Types of Mods To Fix", 5);
        logger->space();

        // The two empties mean opposite things, and this is where a user finds out which one they
        // asked for. No filter at all takes everything; an empty filter takes nothing, which is a
        // run that will do no work -- worth saying out loud rather than reporting as "All mods".
        if (!service.fromModTypeIds.has_value()) {
            logger->log("All mods");
        } else if (service.fromModTypeIds->empty()) {
            logger->log("No mods");
        } else {
            std::vector<std::string> names;
            names.reserve(service.fromModTypeIds->size());

            for (int modTypeId : *service.fromModTypeIds) {
                names.push_back(_modTypeName(modTypeId));
            }

            // By name, not by id: the ids come out of an unordered_set in no order worth showing,
            // and the pure-Python original sorted its names too.
            std::sort(names.begin(), names.end());

            for (const std::string& name : names) {
                logger->bulletPoint(name);
            }
        }

        logger->space();
        logger->closeHeading();
        logger->split();

        logger->includePrefix = hadPrefix;
    }


    std::string RemapServiceCLI::_modTypeName(int modTypeId) {
        // A REGISTERED mod type first, since that is the pure-Python original's own "modType.name"
        // and is the only source that knows a custom type's name.
        const std::optional<ModType> modType = ModTypeIdTools::getModType(modTypeId);
        if (modType.has_value()) {
            return modType->name;
        }

        // Load-bearing, not a courtesy: the registry is filled as a side effect of the first
        // classify(), and this banner prints BEFORE any of that has happened -- so on a normal run
        // every name comes from here. It is the same table (ModTypeNames) the pure-Python side reads.
        const std::optional<ModTypeId> id = ModTypeIdTools::getEnum(modTypeId);
        if (id.has_value()) {
            return ModTypeIdTools::getName(*id);
        }

        // An id belonging to neither -- a custom type the caller never registered. Shown as itself
        // rather than dropped: a missing bullet would read as "this type is not being fixed".
        return std::to_string(modTypeId);
    }


    void RemapServiceCLI::addTips() {
        // Deliberately empty -- see the declaration. Naming a command-line option is the parser's
        // business, and the parser is not in core.
    }

    void RemapServiceCLI::createLog() {
        if (!log_.has_value() || logger == nullptr) {
            return;
        }

        // Without a prefix, and announced before the write: the line naming the log file belongs in
        // the log file, which it only is if it is logged first. Matches the pure-Python original's
        // own ordering.
        const bool hadPrefix = logger->includePrefix;
        logger->includePrefix = false;

        logger->space();
        logger->log("Creating log file, " + FileTypes::Log);

        logger->includePrefix = hadPrefix;

        // The folder may not exist yet -- the user is free to name one the fix never walked into.
        std::error_code err;
        std::filesystem::path path(*log_);
        std::filesystem::create_directories(path.parent_path(), err);

        // Binary mode and an explicit truncate, matching IniFile::write's own: the text this holds
        // was normalized by this codebase, so letting the OS re-translate a newline on the way out
        // would make the file differ between Windows and Linux for no reason.
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) {
            return;
        }

        out << logger->loggedTxt();
    }

    void RemapServiceCLI::_setupLogPath(std::optional<std::string> newLog) {
        if (!newLog.has_value()) {
            log_ = std::nullopt;
            return;
        }

        // A FOLDER comes in and a FILE path goes out -- the pure-Python original's own
        // "os.path.join(self._log, FileTypes.Log.value)". The file's name is never the caller's.
        log_ = (std::filesystem::path(*newLog) / FileTypes::Log).string();
    }
}
