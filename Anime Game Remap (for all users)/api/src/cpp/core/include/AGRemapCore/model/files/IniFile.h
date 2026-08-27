#ifndef AGRemapCore_IniFile_H
#define AGRemapCore_IniFile_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    class BaseIniParser;
    class BaseIniFixer;

    /**
     * @brief
     @rst
     Class to handle ``.ini`` files :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This is a from-scratch C++ port, not a 1-1 translation of the pure-Python
        ``FixRaidenBoss2.model.files.IniFile`` original -- capability is being ported over
        incrementally, so this class is expected to grow additional methods/state over time rather
        than arrive fully-formed in one pass
     @endrst
     */
    class IniFile {
        public:

            /**
             * @brief
             @rst
             Constructs a new .ini file. Does not read anything from disk yet -- see #readFileLines
             @endrst
             *
             * @param file The file path to the .ini file. If this is ``std::nullopt``, 'txt' is
             *      used as the content of the .ini file instead
             * @param txt Used as the text content of the .ini file if 'file' is ``std::nullopt``
             * @param gameTypeId
             @rst
             The specific game to filter classification on, if known -- passed through to
             :cpp:func:`BaseIniClassifier::classify`/:cpp:func:`BaseIniClassifier::checkIsFixedMod`
             as the corresponding :cpp:enum:`GameTypeId`, when 'gameTypeId' maps to one (see
             :cpp:func:`GameTypeIdTools::getEnum`) :raw-html:`<br />` :raw-html:`<br />`

             Stored as a plain ``int`` (not :cpp:enum:`GameTypeId` itself), matching this codebase's
             usual convention of keeping id-shaped config data as plain ints so a custom game id
             not registered in :cpp:enum:`GameTypeId` can still be stored, even though it won't
             actually narrow classification (the classifier's own API only accepts a real
             :cpp:enum:`GameTypeId`) :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param filteredModTypeIds
             @rst
             The specific :cpp:enum:`ModTypeId`\\s (by id) to accept when classifying #modTypes. If
             this is ``std::nullopt``, every :cpp:enum:`ModTypeId` the classifier reports is
             accepted :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param forcedModTypeIds
             @rst
             The specific :cpp:enum:`ModTypeId`\\s (by id) to forcibly use for #modTypes, overriding
             whatever the classifier itself would have determined -- see #classify :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param overrideModTypes
             @rst
             :cpp:class:`ModType`\\s, keyed by their :cpp:enum:`ModTypeId`, that take precedence
             over the global registry (:cpp:func:`ModTypeIdTools::getModType`) when #classify
             resolves a classified/forced :cpp:enum:`ModTypeId` into an actual
             :cpp:class:`ModType` :raw-html:`<br />` :raw-html:`<br />`

             If this is ``std::nullopt``, treated the same as an empty map (nothing overridden)
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param iniClassifier
             @rst
             The classifier used to identify what mod(s) belong to this .ini file :raw-html:`<br />`
             :raw-html:`<br />`

             This is a non-owning pointer to a classifier owned elsewhere -- if this is ``nullptr``,
             uses the shared default classifier from :cpp:func:`GlobalIniClassifiers::classifier`
             instead :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit IniFile(std::optional<std::string> file = std::nullopt, std::string txt = "",
                                  std::optional<int> gameTypeId = std::nullopt,
                                  std::optional<std::unordered_set<int>> filteredModTypeIds = std::nullopt,
                                  std::optional<std::unordered_set<int>> forcedModTypeIds = std::nullopt,
                                  std::optional<std::unordered_map<int, ModType>> overrideModTypes = std::nullopt,
                                  BaseIniClassifier* iniClassifier = nullptr);

            virtual ~IniFile() = default;

            /**
             * @brief The file path to the .ini file, or ``std::nullopt`` if this .ini file has no
             *      backing file and is only ever driven by its in-memory text content
             */
            const std::optional<std::string>& getFile() const;

            /**
             * @brief
             @rst
             The text content of the .ini file, as of the last call to #readFileLines (or the
             constructor's ``txt`` argument, if #getFile is ``std::nullopt``)
             @endrst
             */
            const std::string& getFileTxt() const;

            /**
             * @brief
             @rst
             The text lines of the .ini file, as of the last call to #readFileLines (or the
             constructor's ``txt`` argument, if #getFile is ``std::nullopt``) :raw-html:`<br />`
             :raw-html:`<br />`

             Each line ends with a newline character (``\\n``), except possibly the last line
             @endrst
             */
            const std::vector<std::string>& getFileLines() const;

            /**
             * @brief Whether the lines of the .ini file have been read at least once, either via
             *      #readFileLines or, for a file-less .ini file, the constructor's ``txt`` argument
             */
            bool fileLinesRead() const;

            /**
             * @brief
             @rst
             Reads the lines of the .ini file, similar to Python's `readlines`_ :raw-html:`<br />`
             :raw-html:`<br />`

             If #getFile is ``std::nullopt``, this does not read anything from disk and simply
             returns the existing value of #getFileLines instead :raw-html:`<br />` :raw-html:`<br />`

             .. _readlines: https://docs.python.org/3/tutorial/inputoutput.html#methods-of-file-objects
             @endrst
             *
             * @return The text lines read from the .ini file
             *
             * @throws std::runtime_error if #getFile holds a file path that cannot be opened
             */
            const std::vector<std::string>& readFileLines();

            /**
             * @brief
             @rst
             Classifies the .ini file, determining #isMod, #isFixed, and #modTypes -- reads the
             .ini file first via #readFileLines if it hasn't been read yet (same "read on first use"
             behavior as the pure-Python original's own ``classify``) :raw-html:`<br />`
             :raw-html:`<br />`

             * If the constructor's ``forcedModTypeIds`` argument was ``std::nullopt``, calls the
               classifier's own :cpp:func:`BaseIniClassifier::classify` normally: #isMod/#isFixed are
               set from its result, and #modTypes is built from its ``modType`` map (filtered down
               to the constructor's ``filteredModTypeIds`` argument, when given), resolving each id
               to an actual :cpp:class:`ModType` via the constructor's ``overrideModTypes``/the
               global registry
             * If the constructor's ``forcedModTypeIds`` argument had a value, the classifier is
               never asked to classify a :cpp:enum:`ModTypeId` at all -- only
               :cpp:func:`BaseIniClassifier::checkIsFixedMod` is called, to set #isMod/#isFixed
               independently of any specific mod type, and #modTypes is instead built directly from
               ``forcedModTypeIds`` (each id resolved to a :cpp:class:`ModType` the same way),
               skipping the classifier's own classification entirely
             @endrst
             */
            void classify();

            /**
             * @brief
             @rst
             Retrieves all the :cpp:class:`IfTemplate`\\s for the .ini file, keyed by `section`_ name
             -- same as #readIfTemplates, but caches the result after the first call :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Every :cpp:class:`IfTemplate` returned shares the same :cpp:class:`Z3Context`
                (this instance's own #z3Ctx_) -- matches the pure-Python original's single shared
                ``self._z3Ctx``, and is what lets a caller later combine predicates from different
                `sections`_ of the same .ini file (eg. building an :cpp:class:`IniSectionGraph` over
                them)

             .. note::
                Deliberately not a 1-1 port of the pure-Python original's ``ConfigParser``-backed
                key/value parsing -- see the ``parseSectionKVPs`` doc comment in ``IniFile.cpp``
                (it's a file-local implementation detail, not part of this class) for what's preserved
                (comment/blank-line skipping, original key **case** preserved -- the pure-Python
                original disables `ConfigParser`_'s default key-lowercasing via
                ``self._parser.optionxform=str``, so this does too -- first-``=``-or-``:`` splitting,
                duplicate-key order preservation) versus what's deliberately simplified for speed
                (no ``%``-interpolation, no multi-line continuation -- the pure-Python original
                already strips all leading whitespace from every line before parsing a section,
                which incidentally neuters `ConfigParser`_'s own indentation-based continuation
                feature anyway, so this isn't a real behavior change for any real .ini file)

             .. note::
                A single pass over #fileLines_ builds every section's :cpp:class:`IfTemplate`
                incrementally as section boundaries are discovered, rather than one pass to find
                boundaries followed by a second pass per section to build the parts
             @endrst
             *
             * @param flush Whether to re-parse the :cpp:class:`IfTemplate`\\s instead of using the cached values. **Default**: ``false``
             *
             * @return The parsed :cpp:class:`IfTemplate`\\s, keyed by `section`_ name
             */
            const std::unordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>& getIfTemplates(bool flush = false);

            /**
             * @brief Parses all the :cpp:class:`IfTemplate`\\s for the .ini file -- see #getIfTemplates for the cached version
             *
             * @return The parsed :cpp:class:`IfTemplate`\\s, keyed by `section`_ name
             */
            const std::unordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>& readIfTemplates();

            /**
             * @brief
             @rst
             Whether #classify has been called at least once :raw-html:`<br />` :raw-html:`<br />`

             The direct equivalent of the pure-Python original's own ``self._isClassified`` -- lets
             a caller (eg. :cpp:func:`IniFile::parse`) classify on first use without redundantly
             re-classifying afterwards
             @endrst
             */
            bool isClassified() const;

            /**
             * @brief
             @rst
             The types of mod the .ini file was classified (or forced) as, as of the last call to
             #classify, keyed by their :cpp:enum:`ModTypeId` -- empty if #classify hasn't been
             called, or found nothing :raw-html:`<br />` :raw-html:`<br />`

             The closest equivalent of the pure-Python original's own ``availableType``, except that
             this can hold more than one :cpp:class:`ModType` (see #modTypes)
             @endrst
             */
            const std::unordered_map<int, ModType>& getModTypes() const;

            /**
             * @brief
             @rst
             Sets the fixer used by #fix. Non-owning -- 'fixer' must outlive this object
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike #parse -- which uses each classified :cpp:class:`ModType`'s own
                :cpp:member:`ModType::iniParser` -- #fix still takes a single injected fixer. The
                pure-Python original's ``_getFixer`` builds one from ``availableType.iniFixBuilder``,
                so this is the remaining piece that hasn't been switched over to
                :cpp:member:`ModType::iniFixer` yet
             @endrst
             *
             * @param fixer The fixer to use, or ``nullptr`` to unset it
             */
            void setFixer(BaseIniFixer* fixer);

            /**
             * @brief The fixer used by #fix, or ``nullptr`` if none was set
             */
            BaseIniFixer* getFixer() const;

            /**
             * @brief
             @rst
             Parses the ``.ini`` file once per classified :cpp:class:`ModType`, using each one's own
             :cpp:member:`ModType::iniParser` :raw-html:`<br />` :raw-html:`<br />`

             Follows the same order as the pure-Python original's own ``parse``:

             #. #classify the file, if it hasn't been classified yet
             #. Bail out if the file was classified as no known mod type at all (the equivalent of
                the original's ``if (self.availableType is None): return``)
             #. Refresh the :cpp:class:`IfTemplate`\\s (see the 'flushIfTemplates' argument)
             #. For each entry of #getModTypes, take its :cpp:member:`ModType::iniParser` (skipping
                the mod type entirely if it has none -- the equivalent of the original's
                ``_getParser`` returning ``None``), bind it to this file with
                :cpp:func:`BaseIniParser::setIniFile`, then
                :cpp:func:`BaseIniParser::clear` and :cpp:func:`BaseIniParser::parse` it

             :raw-html:`<br />`

             .. note::
                The pure-Python original only ever has a *single* ``availableType``, so it parses
                once and returns nothing. A C++ :cpp:class:`IniFile` can be classified as several
                :cpp:class:`ModType`\\s at once (see #modTypes), so this parses once per mod type and
                keys the results by :cpp:enum:`ModTypeId`

             .. note::
                Step 4 **rebinds** each :cpp:class:`ModType`'s parser to this file. Because a
                :cpp:class:`ModType` holds one shared parser instance (rather than the pure-Python
                original's per-file factory), two :cpp:class:`IniFile`\\s of the same mod type must
                not be parsed concurrently -- they would stomp each other's binding. See
                :cpp:member:`ModType::iniParser`

             .. note::
                The pure-Python original also clears its ``remapBlendModels``/``remapPositionModels``/
                ``texAddModels``/``texEditModels`` between steps 2 and 3. Those resource-model
                collections don't exist on the C++ :cpp:class:`IniFile` yet, so there's nothing to
                clear -- add it here when they land
             @endrst
             *
             * @param flushIfTemplates Whether to re-parse the :cpp:class:`IfTemplate`\\s instead of using the cached values. **Default**: ``true``
             *
             * @return
             @rst
             The parsed :cpp:class:`IniGraphGroup`\\s, keyed by the :cpp:enum:`ModTypeId` of the
             :cpp:class:`ModType` whose parser produced them -- empty if this bailed out at step 2,
             and missing an entry for any mod type skipped at step 4. Move-only, since
             :cpp:class:`IniGraphGroup` is
             @endrst
             */
            std::unordered_map<int, std::vector<IniGraphGroup>> parse(bool flushIfTemplates = true);

            /**
             * @brief
             @rst
             Fixes the ``.ini`` file, via the fixer set by #setFixer :raw-html:`<br />`
             :raw-html:`<br />`

             Unlike #parse, this does no classifying/parsing of its own first -- call #parse before
             this, the same way the pure-Python original's pipeline does
             @endrst
             *
             * @param keepBackup Whether to keep backups for the .ini file. **Default**: ``true``
             * @param fixOnly Whether to only fix the .ini file without undoing any fixes. **Default**: ``false``
             * @param hideOrig Whether to hide the mod for the original character. **Default**: ``false``
             *
             * @return
             @rst
             The new content of the fixed .ini file(s) keyed by file path, or an empty map if no
             fixer was set
             @endrst
             */
            std::unordered_map<std::string, std::string> fix(bool keepBackup = true, bool fixOnly = false, bool hideOrig = false);

        protected:

            /**
             * @brief Whether the .ini file belongs to a mod, as of the last call to #classify
             */
            bool isMod = false;

            /**
             * @brief Whether the .ini file has already been fixed, as of the last call to #classify
             */
            bool isFixed = false;

            /**
             * @brief
             @rst
             The types of mod the .ini file was classified (or forced) as, as of the last call to
             #classify, keyed by their :cpp:enum:`ModTypeId` :raw-html:`<br />` :raw-html:`<br />`

             Unlike the deprecated pure-Python original (where a .ini file could only ever have a
             single :cpp:class:`ModType`), this can hold more than one entry
             @endrst
             */
            std::unordered_map<int, ModType> modTypes;

        private:
            bool isClassified_ = false;

            // Non-owning -- see setFixer. There's deliberately no parser_ counterpart: parse() uses
            // each classified ModType's own ModType::iniParser instead.
            BaseIniFixer* fixer_ = nullptr;

            std::optional<std::string> file_;
            std::string fileTxt_;
            std::vector<std::string> fileLines_;
            bool fileLinesRead_ = false;

            std::optional<int> gameTypeId_;
            std::optional<std::unordered_set<int>> filteredModTypeIds_;
            std::optional<std::unordered_set<int>> forcedModTypeIds_;
            std::unordered_map<int, ModType> overrideModTypes_;
            BaseIniClassifier* iniClassifier_;

            bool ifTemplatesRead_ = false;
            std::unordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>> sectionIfTemplates_;

            // Shared by every IfTemplate #readIfTemplates produces -- see #getIfTemplates' own doc
            // comment for why this needs to be shared rather than one-fresh-context-per-section.
            Z3Context z3Ctx_;

            // Reads the raw bytes at 'path' as text, normalizing "\r\n"/"\r" line endings down to
            // "\n" (matching Python text-mode's universal newline translation), and stores the
            // result into fileTxt_/fileLines_/fileLinesRead_.
            void readFromDisk(const std::string& path);

            // Splits 'txt' into lines the same way Python's str.splitlines(keepends = True) does --
            // each returned line retains its own trailing "\n", except possibly the last line if
            // 'txt' doesn't end with one. Stores the result into fileTxt_/fileLines_/fileLinesRead_.
            void setFileTxt(std::string txt);

            // Resolves 'modTypeId' to an actual ModType -- overrideModTypes_ takes precedence,
            // falling back to the global registry (ModTypeIdTools::getModType) otherwise.
            // std::nullopt if 'modTypeId' isn't in overrideModTypes_ and isn't registered globally
            // either.
            std::optional<ModType> getModType(int modTypeId) const;

            // Whether 'line' looks like a "[SectionName]" header -- after skipping leading
            // whitespace, starts with '[' and has a ']' somewhere after it. Mirrors the pure-Python
            // original's own '_sectionPattern' (re.compile(r"^\s*\[.*\]")).
            static bool isSectionHeaderLine(const std::string& line);

            // Extracts the section name from a "[SectionName]" header line -- the substring between
            // the first '[' and the last ']' (falling back to a partial extraction if only one
            // bracket is present, or the whole trimmed line if neither is), trimmed of surrounding
            // whitespace. Mirrors the deprecated pure-Python IniClassifierOld.getSectionName.
            static std::string getSectionNameFromLine(const std::string& line);

            // Whether 'line' looks like an if/elif/else/endif conditional line -- after skipping
            // leading whitespace, starts with one of those 4 keywords (case-sensitive, matching the
            // pure-Python original's own uncompiled-with-IGNORECASE '_ifStructurePattern').
            static bool isConditionalLine(const std::string& line);

            // Strips every occurrence of the ";RemapFixHideOrig -->" marker (IniKeywords::
            // HideOriginalComment) from 'line' in place -- matches the pure-Python original's own
            // 'ignoreHideOriginal = True' behavior (always passed by readIfTemplates). A full port
            // would gate this on a '_hideOriginalReplaced' flag set by a not-yet-ported
            // hideOriginalSections(); until that exists, this marker can never actually appear in
            // #fileLines_ in the first place, so stripping it unconditionally is a no-op today and
            // exactly correct once that method lands.
            static void stripHideOriginalComment(std::string& line);
    };
}

#endif
