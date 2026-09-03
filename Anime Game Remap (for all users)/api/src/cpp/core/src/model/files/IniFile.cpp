#include "AGRemapCore/model/files/IniFile.h"

#include <cctype>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <unordered_set>
#include <stdexcept>
#include <utility>
#include <variant>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/FilePrefixes.h"
#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalIniClassifiers.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"
#include "AGRemapCore/constants/GlobalIniRemoveBuilders.h"
#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixingContext.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemovalContext.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"


namespace AGRemapCore {
    IniFile::IniFile(std::optional<std::string> file, std::string txt, std::optional<int> gameTypeId,
                      std::optional<std::unordered_set<int>> filteredFromModTypeIds,
                      std::optional<std::unordered_set<int>> forcedFromModTypeIds,
                      std::optional<std::unordered_map<int, ModType>> overrideModTypes,
                      BaseIniClassifier* iniClassifier,
                      std::optional<ParseData> parseData,
                      DownloadMode downloadMode,
                      std::optional<Version> fromVersion,
                      std::optional<Version> toVersion,
                      std::optional<std::unordered_set<int>> filteredToModTypeIds):
        downloadMode(downloadMode),
        fromVersion(std::move(fromVersion)),
        toVersion(std::move(toVersion)),
        filteredToModTypeIds(std::move(filteredToModTypeIds)),
        file_(std::move(file)),
        gameTypeId_(gameTypeId),
        filteredFromModTypeIds_(std::move(filteredFromModTypeIds)),
        forcedFromModTypeIds_(std::move(forcedFromModTypeIds)),
        overrideModTypes_(overrideModTypes.has_value() ? std::move(*overrideModTypes) : std::unordered_map<int, ModType>()),
        iniClassifier_(iniClassifier != nullptr ? iniClassifier : &GlobalIniClassifiers::classifier()),
        parseData_(std::move(parseData)) {
        // Matches the pure-Python original's own "_setupFileLines" -- a file-backed IniFile isn't
        // read from disk until something actually asks for it (see #readFileLines); a file-less one
        // has no other source of data, so the constructor's 'txt' is used immediately.
        if (!file_.has_value()) {
            setFileTxt(std::move(txt));
        }
    }

    const std::optional<std::string>& IniFile::getFile() const {
        return file_;
    }

    const std::string& IniFile::getFileTxt() const {
        return fileTxt_;
    }

    const std::vector<std::string>& IniFile::getFileLines() const {
        return fileLines_;
    }

    bool IniFile::fileLinesRead() const {
        return fileLinesRead_;
    }

    bool IniFile::isClassified() const {
        return isClassified_;
    }

    const tsl::ordered_map<int, ModType>& IniFile::getModTypes() const {
        return modTypes;
    }

    const std::optional<IniFile::ParseData>& IniFile::getParseData() const {
        return parseData_;
    }

    std::optional<IniFile::ParseData>& IniFile::getParseData() {
        return parseData_;
    }

    void IniFile::clearRead(bool eraseSourceTxt) {
        // Matches the pure-Python original's own 'clearRead': for a file-less .ini file, fileTxt_ is
        // the only source of data there is, so it's kept unless the caller explicitly asks for it to
        // go too.
        if (!file_.has_value() && !eraseSourceTxt) {
            return;
        }

        fileLines_.clear();
        fileTxt_.clear();
        fileLinesRead_ = false;
        isFixed = false;
    }

    void IniFile::clear(bool eraseSourceTxt) {
        clearRead(eraseSourceTxt);

        isMod = false;
        isFixed = false;
        modTypes.clear();
        isClassified_ = false;

        ifTemplatesRead_ = false;

        // Order matters: the IfTemplates hold predicates built against z3Ctx_, so they have to go
        // before it is replaced.
        sectionIfTemplates_.clear();

        // The pure-Python original's clear() ends with clearModels(), which empties ini.resources
        // and ini.fileDownloads among others. Same thing here -- and see getResources' own danger
        // note on what that does to a resource edit still holding pointers into them.
        clearModels();

        // A fresh context rather than an in-place clear, exactly as the pure-Python original does
        // ("self._z3Ctx = Z3Context()"). Z3Context is move-assignable but not copyable.
        z3Ctx_ = Z3Context();

        parseData_.reset();

        // Matches the pure-Python original's own "self._iniParser = None" / "self._iniFixer = None"
        // in clear(). Both are bound to this file's now-discarded state, so neither can be reused --
        // the next parse()/fix() builds fresh ones.
        //
        // Order matters: a built fixer holds a non-owning pointer into its built parser
        // (BaseIniFixer::getParser), so the fixers have to go first.
        builtFixers_.clear();
        builtParsers_.clear();
    }

    BaseIniParser<>* IniFile::getParser(int modTypeId, ModType& modType) {
        // Keyed by the id 'modTypes' filed this ModType under, NOT by ModType::modTypeId -- the
        // constructor's 'overrideModTypes' argument is free to file a ModType under an id its own
        // modTypeId doesn't match (the classify tests rely on exactly that), and the parse data
        // this parser feeds is keyed the same way.
        auto it = builtParsers_.find(modTypeId);
        if (it != builtParsers_.end()) {
            return it->second.get();
        }

        // The equivalent of the pure-Python original's "_getParser" returning None -- a mod type
        // with no builder at all simply contributes nothing.
        if (modType.iniParseBuilder == nullptr) {
            return nullptr;
        }

        // The whole point of holding a builder rather than a parser: 'fromVersion' is this file's
        // own, so two .ini files of the same mod type but different game versions get the parser
        // each one actually needs. Mirrors the original's
        // "iniParseBuilder.build(self, modName = ..., version = self.version)".
        // 'modTypeId' rather than modType.modTypeId: the id this ModType was FILED under, which
        // overrideModTypes is free to make differ -- see this method's own note above.
        std::shared_ptr<BaseIniParser<>> parser =
            modType.iniParseBuilder->build(this, modType.name, fromVersion, modTypeId);
        if (parser == nullptr) {
            return nullptr;
        }

        return builtParsers_.emplace(modTypeId, std::move(parser)).first->second.get();
    }

    const std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>>& IniFile::getFixers(int modTypeId, ModType& modType) {
        auto it = builtFixers_.find(modTypeId);
        if (it != builtFixers_.end()) {
            return it->second;
        }

        // Cached even when the answer is "none", so a repeat fix() doesn't re-run the lookup. The
        // empty vector is a legitimate result, not a "not computed yet" marker.
        std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>> fixers;

        if (modType.iniFixBuilder != nullptr) {
            // A fixer is built *from the parser*, not from the file -- so a mod type whose parser
            // could not be built gets no fixers either. That mirrors the pure-Python original's
            // '_getFixer', which is guarded on "self._iniParser is not None" for exactly this
            // reason.
            BaseIniParser<>* parser = getParser(modTypeId, modType);
            if (parser != nullptr) {
                // One fixer per target mod: the (fromVersion, fromModName, toVersion) triple is
                // held fixed and the target mod is the axis fanned out over. filteredToModTypeIds
                // narrows which targets are built; std::nullopt means all of them.
                //
                // The filter is held as ids but IniFixBuilder's table is keyed by mod NAME, so it
                // is resolved here rather than there -- id -> name is an exact switch
                // (ModTypeIdTools::getName), whereas name -> id would have to go through the
                // fuzzy, maximal-match findByName. An id no ModTypeId recognizes simply
                // contributes no name, which is what makes it match nothing.
                std::optional<std::unordered_set<std::string>> filteredToModNames;
                if (filteredToModTypeIds.has_value()) {
                    filteredToModNames.emplace();
                    for (int toModTypeId : *filteredToModTypeIds) {
                        std::optional<ModTypeId> resolved = ModTypeIdTools::getEnum(toModTypeId);
                        if (resolved.has_value()) {
                            filteredToModNames->insert(ModTypeIdTools::getName(*resolved));
                        }
                    }
                }

                fixers = modType.iniFixBuilder->buildAll(parser, modType.name, fromVersion, toVersion,
                                                          filteredToModNames);
            }
        }

        return builtFixers_.emplace(modTypeId, std::move(fixers)).first->second;
    }

    std::vector<IniGraphGroup<>> IniFile::parseModType(int modTypeId, ModType& modType) {
        BaseIniParser<>* parser = getParser(modTypeId, modType);
        if (parser == nullptr) {
            return {};
        }

        // No setIniFile here, unlike the earlier shared-parser design: the builder hands back a
        // parser already bound to this file.
        parser->clear();
        return parser->parse();
    }

    IniFile::ParseData& IniFile::parse(bool flushIfTemplates) {
        if (!isClassified_) {
            classify();
        }

        // Parsing has now happened, even if it finds nothing -- so the result is always a present
        // (if possibly empty) optional afterwards. That present-but-empty state is what stops fix()
        // from re-parsing.
        ParseData& result = parseData_.emplace();

        // The equivalent of the pure-Python original's "if (self.availableType is None): return"
        // -- nothing recognized this .ini file as belonging to any mod, so there's nothing to parse.
        if (modTypes.empty()) {
            return result;
        }

        getIfTemplates(flushIfTemplates);

        // The pure-Python original has exactly one 'availableType' and so parses exactly once; a
        // C++ IniFile can be classified as several ModTypes at once, so each one's own parser runs
        // and its results are keyed by that mod type's id.
        for (auto& entry : modTypes) {
            if (entry.second.iniParseBuilder == nullptr) {
                // That mod type simply contributes no entry, rather than aborting the whole parse.
                continue;
            }

            // tsl::ordered_map hands out const values through its iterators -- it keeps them in a
            // contiguous vector and will not let one be mutated behind the hash index. at() is the
            // mutable accessor, and the strategies below take a ModType&.
            result.emplace(entry.first, parseModType(entry.first, modTypes.at(entry.first)));
        }

        return result;
    }

    std::unordered_map<std::string, std::string> IniFile::fix(bool keepBackup, bool fixOnly, bool hideOrig) {
        if (!isClassified_) {
            classify();
        }

        std::unordered_map<std::string, std::string> result;
        if (modTypes.empty()) {
            return result;
        }

        if (!parseData_.has_value()) {
            parseData_.emplace();
        }

        // Split in two on purpose. A fixer has to be told whether its mod type is the file's first
        // and/or its last (see IniFixingContext) -- the first takes the backup, the last hides the
        // original mod -- and "which mod types actually contribute a fixer" is not knowable without
        // asking every one of them first. So this pass parses and builds, and the pass below fixes.
        //
        // Safe to separate because nothing in the first pass depends on the second: parsing works
        // off the section templates read once up front, not off the file on disk, so a fix moving
        // that file aside or writing next to it cannot change what a later mod type parses.
        //
        // The pointers below are into two unordered_maps that keep growing here, which is fine --
        // std::unordered_map only invalidates *iterators* on rehash, never references to elements.
        using Fixers = std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>>;
        std::vector<std::pair<std::vector<IniGraphGroup<>>*, const Fixers*>> pending;

        for (auto& entry : modTypes) {
            int modTypeId = entry.first;
            // See parseData()'s note: at() is tsl::ordered_map's mutable accessor.
            ModType& modType = modTypes.at(entry.first);

            // Reuse this mod type's existing parse data when there is some, otherwise parse it now
            // and cache it, so a later parse()/fix() doesn't redo the work. Done before getFixer()
            // so the parser it binds to is the same one that produced this data.
            auto parsedIt = parseData_->find(modTypeId);
            if (parsedIt == parseData_->end()) {
                getIfTemplates();
                parsedIt = parseData_->emplace(modTypeId, parseModType(modTypeId, modType)).first;
            }

            // One fixer per target mod this source mod fixes to -- Jean, for instance, fixes to both
            // JeanCN and JeanSea, so both run against the same parse data. An empty list is the
            // equivalent of the original's "_getFixer" returning None (no fix builder, no parser to
            // build against, or every target filtered out by filteredToModTypeIds): that mod type
            // simply contributes nothing rather than aborting the whole fix.
            const Fixers& fixers = getFixers(modTypeId, modType);

            bool contributes = false;
            for (const std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>& fixerEntry : fixers) {
                if (fixerEntry.second != nullptr) {
                    contributes = true;
                    break;
                }
            }

            // Left out of 'pending' entirely rather than carried and skipped: a mod type that runs
            // nothing must not be able to claim the file's first or last word.
            if (contributes) {
                pending.emplace_back(&parsedIt->second, &fixers);
            }
        }

        // "First" and "last" are iteration order over an unordered_map, which is arbitrary but
        // stable for a given container, and that is all this needs: the fixers chain over the same
        // file, so which mod type takes the backup and which one hides matters only in that exactly
        // one of them does each.
        //
        // Both flags are per *mod type*, so a mod type fixing to several target mods gives all of
        // its fixers the same answer -- they write to the same path anyway, and the last of them
        // wins there.
        std::size_t pendingCount = pending.size();

        for (std::size_t i = 0; i < pendingCount; ++i) {
            IniFixingContext fixingCtx(i == 0, i + 1 == pendingCount);

            for (const std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>& fixerEntry : *pending[i].second) {
                if (fixerEntry.second == nullptr) {
                    continue;
                }

                BaseIniFixer<>::FixResult modTypeFix =
                    fixerEntry.second->fix(*pending[i].first, keepBackup, fixOnly, hideOrig, fixingCtx);

                // Combine into the running result. A plain overwrite -- see this method's doc
                // comment on what happens when two fixers target the same file path.
                for (auto& fixEntry : modTypeFix) {
                    result[fixEntry.first] = std::move(fixEntry.second);
                }
            }
        }

        return result;
    }

    std::vector<std::string> IniFile::getSectionNames() {
        std::vector<std::string> result;

        // getIfTemplates rather than sectionIfTemplates_ directly: this has to read the file first
        // if it has not been read, exactly as that method does.
        const auto& sections = getIfTemplates();
        result.reserve(sections.size());

        // A tsl::ordered_map, so this really is declaration order -- see getSectionNames' own note.
        for (const auto& entry : sections) {
            result.push_back(entry.first);
        }

        return result;
    }


    IfTemplate<std::string, std::string>* IniFile::getSection(const std::string& name) {
        const auto& sections = getIfTemplates();
        auto it = sections.find(name);

        if (it == sections.end()) {
            return nullptr;
        }

        return it->second.get();
    }


    IfTemplate<std::string, std::string>* IniFile::addSection(const std::string& name,
                                                              std::unique_ptr<IfTemplate<std::string, std::string>> section) {
        // Read first, so adding to an unread file doesn't get overwritten by a later read.
        getIfTemplates();

        IfTemplate<std::string, std::string>* result = section.get();

        auto it = sectionIfTemplates_.find(name);
        if (it != sectionIfTemplates_.end()) {
            // Assigned rather than erased-and-reinserted: tsl::ordered_map keeps an existing key
            // where it already was, which is what "replacing" should mean here.
            it.value() = std::move(section);
            return result;
        }

        sectionIfTemplates_.emplace(name, std::move(section));
        return result;
    }


    void IniFile::removeSection(const std::string& name) {
        getIfTemplates();

        // tsl::ordered_map::erase keeps the surviving keys in order (unlike unordered_erase, which
        // swaps the last one into the hole) -- and that order is load-bearing here.
        sectionIfTemplates_.erase(name);
    }


    const std::vector<std::unique_ptr<IniResource>>& IniFile::getFileDownloads() const {
        return fileDownloads_;
    }


    std::vector<std::unique_ptr<IniResource>>& IniFile::getFileDownloads() {
        return fileDownloads_;
    }


    std::optional<std::string> IniFile::disableIni(bool makeCopy) {
        if (!file_.has_value()) {
            return std::nullopt;
        }

        std::filesystem::path path(*file_);
        std::error_code err;

        if (!std::filesystem::exists(path, err) || err) {
            return std::nullopt;
        }

        // Prefix on the name and a .txt extension, matching FileService.disableFile exactly -- the
        // extension change is what stops a mod loader from reading it as a .ini file at all.
        std::filesystem::path backup = path.parent_path() /
            (FilePrefixes::BackupFilePrefix + path.stem().string() + FileExt::Txt);

        std::filesystem::rename(path, backup, err);
        if (err) {
            return std::nullopt;
        }

        if (makeCopy) {
            std::filesystem::copy_file(backup, path, std::filesystem::copy_options::overwrite_existing, err);
        }

        return backup.string();
    }


    const std::vector<std::unique_ptr<IniResource>>& IniFile::getResources() const {
        return resources_;
    }


    std::vector<std::unique_ptr<IniResource>>& IniFile::getResources() {
        return resources_;
    }


    void IniFile::clearModels() {
        resources_.clear();
        fileDownloads_.clear();
    }


    std::vector<std::string> IniFile::getReferencedFolders() const {
        // Insertion-ordered rather than an unordered_set: the pure-Python original uses an
        // OrderedSet here, and the result feeds a user-facing listing of the folders a mod
        // touches -- a shuffled one would read as nondeterministic output.
        std::vector<std::string> result;
        std::unordered_set<std::string> seen;

        auto addFrom = [&result, &seen](const std::vector<std::unique_ptr<IniResource>>& resources) {
            for (const std::unique_ptr<IniResource>& resource : resources) {
                if (resource == nullptr) {
                    continue;
                }

                // 'srcPath' is already absolute -- IniResource's constructor resolves it against
                // the .ini file's own folder -- so no second resolution is needed here.
                std::string folder = std::filesystem::path(resource->srcPath).parent_path().string();
                if (seen.insert(folder).second) {
                    result.push_back(std::move(folder));
                }
            }
        };

        addFrom(resources_);
        addFrom(fileDownloads_);

        return result;
    }


    Z3Context* IniFile::getZ3Ctx() {
        return &z3Ctx_;
    }


    std::string IniFile::getFolder() const {
        if (!file_.has_value()) {
            return "";
        }

        // The same derivation every core-side context already does for its own iniFolder() -- see
        // IniFileRemoveContext::iniFolder.
        return std::filesystem::path(*file_).parent_path().string();
    }


    bool IniFile::getIsMod() const {
        return isMod;
    }


    bool IniFile::getIsFixed() const {
        return isFixed;
    }


    void IniFile::setIsFixed(bool newIsFixed) {
        isFixed = newIsFixed;
    }


    const ModType* IniFile::getAvailableType() const {
        if (modTypes.empty()) {
            // Nothing classified -- fall back to defaultModTypeId, which is the whole point of it.
            return resolveDefaultModType();
        }

        // First in iteration order -- arbitrary but stable, and only meaningful at all when there
        // is exactly one. See this method's own danger note.
        return &modTypes.begin()->second;
    }


    const ModType* IniFile::resolveDefaultModType() const {
        if (!defaultModTypeId.has_value()) {
            return nullptr;
        }

        if (defaultModTypeCachedId_ != defaultModTypeId) {
            defaultModType_ = getModType(*defaultModTypeId);
            defaultModTypeCachedId_ = defaultModTypeId;
        }

        return defaultModType_.has_value() ? &(*defaultModType_) : nullptr;
    }


    std::string IniFile::removeFix(bool parse, bool writeBack) {
        // The pure-Python original leans on its own "_readLines" decorator for this, per remover;
        // doing it once up front here is the same guarantee without the decorator machinery.
        if (!fileLinesRead_) {
            readFileLines();
        }

        if (!isClassified_) {
            classify();
        }

        // Nothing ran, so the file's content is whatever it already was.
        std::string result = fileTxt_;

        // Which mod type gets the last word, resolved before the loop rather than inside it. Every
        // pass but that one asks RemapIniRemover's strict question ("is this section mine?"); the last
        // one is handed IniRemovalContext::ignoreModType and sweeps up whatever is still standing --
        // see that member for why the strict rule alone would leave debris behind forever.
        //
        // "Last" is iteration order over an unordered_map, which is arbitrary but stable for a given
        // container, and that is all this needs: the passes chain over the same file, so which one
        // draws the sweep matters only in that exactly one does and it goes last.
        ModType* sweeper = nullptr;
        for (auto& entry : modTypes) {
            if (entry.second.iniRemoveBuilder != nullptr) {
                // See parseData()'s note: at() is tsl::ordered_map's mutable accessor.
                sweeper = &modTypes.at(entry.first);
            }
        }

        // An unclassified file has no mod type to ask, so it falls back to the global builder for
        // one sweeping pass -- the pure-Python original's own "availableType is None" branch in
        // _getRemover. Being the only pass makes it the last one, hence the sweep.
        //
        // Deliberately keyed on "no mod types at all" rather than on "no mod type offered a
        // remover": a ModType whose iniRemoveBuilder was explicitly nulled is saying it has nothing
        // to contribute here, and overriding that with the global remover would be ignoring it.
        if (modTypes.empty()) {
            const std::shared_ptr<IniRemoveBuilder>& fallback = GlobalIniRemoveBuilders::removeBuilder();
            if (fallback == nullptr) {
                return result;
            }

            std::shared_ptr<BaseIniRemover<>> remover = fallback->build(this, "", fromVersion);
            if (remover == nullptr) {
                return result;
            }

            return remover->remove(parse, writeBack, IniRemovalContext(true));
        }

        for (auto& entry : modTypes) {
            // That mod type simply contributes nothing, rather than aborting the whole removal.
            if (entry.second.iniRemoveBuilder == nullptr) {
                continue;
            }

            // Built fresh per call rather than cached into a builtRemovers_ map the way parsers
            // and fixers are -- see removeFix's own doc comment. Nothing here reads the remover
            // back afterwards, so there is nothing to keep.
            std::shared_ptr<BaseIniRemover<>> remover =
                entry.second.iniRemoveBuilder->build(this, entry.second.name, fromVersion);
            if (remover == nullptr) {
                continue;
            }

            // The removers chain: each one strips its own mod type's fix out of the same file, so
            // the last one's return value is the file's final content.
            result = remover->remove(parse, writeBack, IniRemovalContext(&entry.second == sweeper));
        }

        return result;
    }

    void IniFile::setFileTxt(std::string txt) {
        fileTxt_ = std::move(txt);
        fileLines_.clear();

        // Split fileTxt_ into lines the same way Python's str.splitlines(keepends = True) does --
        // walk the text once, cutting a new line every time a '\n' is found and keeping it attached
        // to the line it ends. A trailing '\n' terminates the last line rather than starting a new,
        // empty one (eg. "abc\n" -> ["abc\n"], not ["abc\n", ""]) -- the loop below naturally does
        // this since it only ever starts a new line when there's still text left to put in it.
        size_t lineStart = 0;
        while (lineStart < fileTxt_.size()) {
            size_t newlinePos = fileTxt_.find('\n', lineStart);
            if (newlinePos == std::string::npos) {
                fileLines_.emplace_back(fileTxt_.substr(lineStart));
                break;
            }

            fileLines_.emplace_back(fileTxt_.substr(lineStart, newlinePos - lineStart + 1));
            lineStart = newlinePos + 1;
        }

        fileLinesRead_ = true;
    }

    std::string IniFile::write(std::optional<std::string> txt) {
        // A file-less .ini file has nowhere to write to, so 'txt' is simply adopted as the new
        // content instead -- the pure-Python original's own first branch.
        if (!file_.has_value()) {
            if (txt.has_value()) {
                setFileTxt(std::move(*txt));
            }

            return fileTxt_;
        }

        // Deliberately NOT routed through setFileTxt: the original only updates its in-memory text
        // on the file-less branch above, so a caller writing an explicit 'txt' to a real path
        // leaves getFileTxt() alone. See this method's own note in the header.
        const std::string& content = txt.has_value() ? *txt : fileTxt_;

        // Binary mode, matching readFromDisk's own: the newline normalization this class does is
        // its own (see readFromDisk), so letting the OS re-translate a written newline back into a
        // carriage-return pair here would make a written-then-read round trip lossy on Windows and
        // not on Linux.
        std::ofstream out(*file_, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("Unable to open file for writing: " + *file_);
        }

        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        out.close();

        return content;
    }


    void IniFile::readFromDisk(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + path);
        }

        std::ostringstream buf;
        buf << file.rdbuf();
        std::string txt = buf.str();

        // Normalize "\r\n"/lone "\r" line endings down to "\n", matching the universal-newline
        // translation Python's text-mode "open(path, 'r')" performs on read -- done manually (rather
        // than relying on the ifstream/OS text-mode behavior above) so this is portable and behaves
        // the same on every platform, not just Windows.
        std::string normalized;
        normalized.reserve(txt.size());
        for (size_t i = 0; i < txt.size(); ++i) {
            char c = txt[i];
            if (c == '\r') {
                normalized.push_back('\n');
                if (i + 1 < txt.size() && txt[i + 1] == '\n') {
                    ++i;
                }
            } else {
                normalized.push_back(c);
            }
        }

        setFileTxt(std::move(normalized));
    }

    const std::vector<std::string>& IniFile::readFileLines() {
        if (file_.has_value()) {
            readFromDisk(*file_);
        }
        return fileLines_;
    }

    std::optional<ModType> IniFile::getModType(int modTypeId) const {
        auto overrideIt = overrideModTypes_.find(modTypeId);
        if (overrideIt != overrideModTypes_.end()) {
            return overrideIt->second;
        }

        // ModTypeIdTools::getModType now takes the raw int directly (no more ModTypeId-enum-only
        // restriction), so a genuinely custom 'modTypeId' (one ModTypeIdTools::getEnum wouldn't
        // recognize) can be resolved via the global registry too now, not just overrideModTypes_
        // above -- as long as it was actually registered there via registerModType.
        return ModTypeIdTools::getModType(modTypeId);
    }

    void IniFile::classify() {
        if (!fileLinesRead_) {
            readFileLines();
        }

        // gameTypeId_ is kept as a plain int (see the constructor's own doc comment on why) -- the
        // classifier's API only understands a real GameTypeId, so an unrecognized custom id simply
        // can't be expressed there and falls back to "unfiltered" (std::nullopt) for classification
        // purposes.
        std::optional<GameTypeId> gameTypeIdEnum = gameTypeId_.has_value() ? GameTypeIdTools::getEnum(*gameTypeId_) : std::nullopt;

        modTypes.clear();

        if (forcedFromModTypeIds_.has_value()) {
            bool isFixedResult = false;
            bool isModResult = false;
            iniClassifier_->checkIsFixedMod(fileLines_, &isFixedResult, &isModResult, gameTypeIdEnum);
            isFixed = isFixedResult;
            isMod = isModResult;

            for (int modTypeId : *forcedFromModTypeIds_) {
                std::optional<ModType> modType = getModType(modTypeId);
                if (modType.has_value()) {
                    modTypes.emplace(modTypeId, *modType);
                }
            }

            // Set at each exit rather than up front, so a classifier that throws part-way leaves
            // this false -- an aborted classify hasn't classified anything.
            isClassified_ = true;
            return;
        }

        IniClassifyStats stats = iniClassifier_->classify(fileLines_, gameTypeIdEnum);
        isMod = stats.isMod;
        isFixed = stats.isFixed;

        for (const auto& entry : stats.modType) {
            int modTypeId = entry.first;
            if (filteredFromModTypeIds_.has_value() && !filteredFromModTypeIds_->contains(modTypeId)) {
                continue;
            }

            std::optional<ModType> modType = getModType(modTypeId);
            if (modType.has_value()) {
                modTypes.emplace(modTypeId, *modType);
            }
        }

        // Mirrors the pure-Python original's own '_isModIni' expression: a filter was supplied and
        // nothing survived it, so this is not a mod of any type the caller cares about -- unless a
        // fallback type stands in, in which case the classifier's own answer is kept.
        if (!defaultModTypeId.has_value() && modTypes.empty()
                && filteredFromModTypeIds_.has_value() && !filteredFromModTypeIds_->empty()) {
            isMod = false;
        }

        isClassified_ = true;
    }

    namespace {
        // Trims ASCII whitespace (space/tab/CR/LF) from both ends of 'str' -- not a member, just a
        // small local helper; matches the ' '/'\t' trimming ConfigParser itself performs on keys and
        // values.
        std::string trim(const std::string& str) {
            size_t start = 0;
            while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
                ++start;
            }

            if (start == str.size()) {
                return "";
            }

            size_t end = str.size();
            while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
                --end;
            }

            return str.substr(start, end - start);
        }

        // NOTE: no key-lowercasing helper here -- IniFile.py sets
        // "self._parser.optionxform=str" to disable ConfigParser's default lowercasing, so keys
        // keep their original case (see parseSectionKVPs below).

        // The parsed KVPs of one content chunk, keyed by KVP key -- every value a duplicated key
        // takes is kept, in encounter order, as (orderInd, value) pairs.
        using SectionKVPs = tsl::ordered_map<std::string, std::vector<std::pair<long long, std::string>>>;

        // One entry in the argument IfTemplate::build expects: either a raw conditional line
        // ("if ...", "else", "endif") or a parsed block of KVPs, tagged with its 1-based line number.
        using RawPart = std::pair<int, std::variant<std::string, SectionKVPs>>;

        /*
         * Parses one section's raw KVP text into the shape IfTemplate::build expects for a content
         * part -- a purpose-built single-pass parser, not a port of Python's ConfigParser.
         *
         * Preserved: blank-line and full-line-comment (';'/'#' prefix) skipping; splitting each line
         * on the first '=' or ':'; trimming whitespace around both key and value; original key CASE
         * (the pure-Python original disables ConfigParser's default key-lowercasing 'optionxform'
         * via "self._parser.optionxform=str", so keys are NOT lowercased here either -- confirmed
         * against the real Python original, not just assumed from ConfigParser's documented
         * default); preserving EVERY value for a duplicate key, in encounter order, as
         * (orderInd, value) pairs sharing one section-wide incrementing counter (the direct C++
         * equivalent of what the pure-Python original's KeepAllDict + "{index}_{value}"-string-
         * encoding trick works around -- this builds the same (index, value) shape natively, with no
         * encode/decode round trip).
         *
         * Not preserved (by explicit user direction -- "you don't have to follow exactly what
         * ConfigParser does... I value speed"): '%'-style interpolation (values are taken literally,
         * so a bare '%' never raises); multi-line continuation lines (moot in practice -- the
         * pure-Python original already strips all leading whitespace from every line before parsing
         * a section, which incidentally neuters ConfigParser's own indentation-based continuation
         * feature anyway); ConfigParser's stricter parse-error behavior (a malformed line with no
         * '='/':' is silently skipped here, rather than aborting the whole section).
         */
        SectionKVPs parseSectionKVPs(const std::string& srcTxt) {
            SectionKVPs result;
            long long orderInd = 0;

            size_t lineStart = 0;
            size_t srcLen = srcTxt.size();

            while (lineStart <= srcLen) {
                size_t newlinePos = srcTxt.find('\n', lineStart);
                std::string rawLine = (newlinePos == std::string::npos) ? srcTxt.substr(lineStart) : srcTxt.substr(lineStart, newlinePos - lineStart);

                std::string line = trim(rawLine);
                if (!line.empty() && line[0] != ';' && line[0] != '#') {
                    size_t delimPos = line.find_first_of("=:");
                    if (delimPos != std::string::npos) {
                        // NOTE: real ConfigParser normally lowercases option (key) names via its
                        // 'optionxform' hook, but IniFile.py explicitly overrides that to the identity
                        // function ("self._parser.optionxform=str") -- so keys keep their original
                        // case here too. Confirmed against the real Python original: 'Filename' stays
                        // 'Filename', it is NOT lowered to 'filename'.
                        std::string key = trim(line.substr(0, delimPos));
                        std::string value = trim(line.substr(delimPos + 1));

                        result[key].emplace_back(orderInd, std::move(value));
                        ++orderInd;
                    }
                    // A line with no '=' or ':' delimiter (and not blank/a comment) has no direct
                    // ConfigParser-continuation-line equivalent here (see this function's own doc
                    // comment on why continuation lines are moot in practice) -- silently skipped
                    // rather than aborting the whole section's parse, unlike real ConfigParser.
                }

                if (newlinePos == std::string::npos) {
                    break;
                }
                lineStart = newlinePos + 1;
            }

            return result;
        }

        // The in-progress state for the one section readIfTemplates is currently walking. Grouped
        // into a struct purely so finalizeSection below can take it as a single parameter instead of
        // the 6 separate by-reference arguments it would otherwise need.
        struct SectionAccum {
            // The section's name, from its own "[SectionName]" header line.
            std::string name;

            // The index into fileLines_ of that header line itself.
            size_t startInd = 0;

            // The parts completed so far -- conditional lines, and KVP blocks already flushed.
            std::vector<RawPart> rawParts;

            // Content lines seen since the last flush, not yet parsed into a RawPart.
            std::string pendingKvpText;

            // The 1-based line number 'pendingKvpText' starts at.
            size_t pendingStartLineNo = 0;

            // Whether 'pendingKvpText' has had anything appended since the last flush. Distinct from
            // "pendingKvpText is non-empty" -- a run of blank lines appends only "\n"s, which is
            // still a pending chunk that a following conditional line must flush.
            bool hasPending = false;
        };

        /*
         * Finishes building the IfTemplate for the section 'accum' has been accumulating, and stores
         * it into 'sectionIfTemplates' -- but only if this is that section name's FIRST occurrence
         * (matching readIfTemplates' own 'handleDuplicateFunc = lambda duplicates: duplicates[0]').
         * Any still-pending KVP text is flushed first, and -- unlike the mid-scan flush inside
         * readIfTemplates' loop -- only appended if it actually parses to something non-empty
         * (matching the pure-Python original's own "if (currentPart):" check). Resets 'accum's
         * part state afterwards so the next section starts clean.
         *
         * Called once per section boundary as those boundaries are discovered during readIfTemplates'
         * single pass. This replaces what used to be a wholly separate second pass (the old
         * IniFile::buildIfTemplate, which re-walked fileLines_[startInd, endInd) for every section
         * after readIfTemplates had already walked the whole file once just to find those boundaries).
         */
        void finalizeSection(SectionAccum& accum,
                             tsl::ordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>& sectionIfTemplates,
                             const std::optional<std::string>& file,
                             Z3Context& z3Ctx) {
            if (!accum.pendingKvpText.empty()) {
                SectionKVPs parsed = parseSectionKVPs(accum.pendingKvpText);
                if (!parsed.empty()) {
                    accum.rawParts.emplace_back(static_cast<int>(accum.pendingStartLineNo), std::move(parsed));
                }
            }

            if (sectionIfTemplates.find(accum.name) == sectionIfTemplates.end()) {
                ParseContext ctx("", file, accum.startInd + 1);
                IfTemplateRunConfig<std::string, std::string> runConfig{
                    IniKeywords::Run,
                    [](const std::string& v) { return v; },
                    [](const std::string& s) { return s; }
                };
                sectionIfTemplates.emplace(accum.name,
                    IfTemplate<std::string, std::string>::build(accum.rawParts, runConfig, accum.name, &ctx, &z3Ctx));
            }

            accum.rawParts.clear();
            accum.pendingKvpText.clear();
            accum.hasPending = false;
        }
    }

    bool IniFile::isSectionHeaderLine(const std::string& line) {
        size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) {
            ++i;
        }

        if (i >= line.size() || line[i] != '[') {
            return false;
        }

        return line.find(']', i) != std::string::npos;
    }

    std::string IniFile::getSectionNameFromLine(const std::string& line) {
        size_t leftPos = line.find('[');
        size_t rightPos = line.rfind(']');
        std::string name;

        if (leftPos != std::string::npos && rightPos != std::string::npos && rightPos > leftPos) {
            name = line.substr(leftPos + 1, rightPos - leftPos - 1);
        } else if (rightPos != std::string::npos) {
            name = line.substr(0, rightPos);
        } else if (leftPos != std::string::npos) {
            name = line.substr(leftPos + 1);
        } else {
            name = line;
        }

        return trim(name);
    }

    bool IniFile::isConditionalLine(const std::string& line) {
        size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) {
            ++i;
        }

        // Checked in the same order as the pure-Python original's own compiled pattern
        // ("endif|else|if|elif") -- order doesn't actually matter for correctness here (none of
        // these 4 keywords is a prefix of another), kept only for easy comparison against that
        // source pattern.
        static const std::string keywords[] = {"endif", "else", "if", "elif"};
        for (const std::string& keyword : keywords) {
            if (line.compare(i, keyword.size(), keyword) == 0) {
                return true;
            }
        }

        return false;
    }

    void IniFile::stripHideOriginalComment(std::string& line) {
        StringTools::eraseAll(line, IniKeywords::HideOriginalComment);
    }

    const tsl::ordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>& IniFile::readIfTemplates() {
        if (!fileLinesRead_) {
            readFileLines();
        }

        sectionIfTemplates_.clear();

        bool inSection = false;
        SectionAccum accum;

        size_t fileLinesLen = fileLines_.size();

        for (size_t i = 0; i < fileLinesLen; ++i) {
            std::string line = fileLines_[i];
            stripHideOriginalComment(line);

            bool isHeader = isSectionHeaderLine(line);

            if (isHeader) {
                if (inSection) {
                    finalizeSection(accum, sectionIfTemplates_, file_, z3Ctx_);
                }

                accum.name = getSectionNameFromLine(line);
                accum.startInd = i;
                inSection = true;
                continue;
            }

            if (!inSection) {
                continue;
            }

            bool isConditional = isConditionalLine(line);

            // Mirrors the pure-Python original's own '_processIfTemplate': flushing here is
            // unconditional (even an empty parse result gets appended) -- only the tail flush inside
            // finalizeSection is gated on a non-empty result.
            if (isConditional && accum.hasPending) {
                accum.rawParts.emplace_back(static_cast<int>(accum.pendingStartLineNo), parseSectionKVPs(accum.pendingKvpText));
                accum.pendingKvpText.clear();
                accum.hasPending = false;
            }

            if (isConditional) {
                accum.rawParts.emplace_back(static_cast<int>(i + 1), line);
                continue;
            }

            accum.pendingKvpText += line;
            if (!accum.hasPending) {
                accum.pendingStartLineNo = i + 1;
            }
            accum.hasPending = true;
        }

        if (inSection) {
            finalizeSection(accum, sectionIfTemplates_, file_, z3Ctx_);
        }

        ifTemplatesRead_ = true;
        return sectionIfTemplates_;
    }

    const tsl::ordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>& IniFile::getIfTemplates(bool flush) {
        if (!ifTemplatesRead_ || flush) {
            readIfTemplates();
        }
        return sectionIfTemplates_;
    }
}
