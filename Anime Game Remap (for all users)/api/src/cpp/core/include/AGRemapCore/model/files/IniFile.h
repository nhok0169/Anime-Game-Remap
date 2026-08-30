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

#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    class BaseIniRemover;

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
             What #parse produces, and what #getParseData caches -- the parsed
             :cpp:class:`IniGraphGroup`\s keyed by the :cpp:enum:`ModTypeId` of the
             :cpp:class:`ModType` whose parser produced them :raw-html:`<br />` :raw-html:`<br />`

             Move-only, since :cpp:class:`IniGraphGroup` is
             @endrst
             */
            using ParseData = std::unordered_map<int, std::vector<IniGraphGroup<>>>;

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
             * @param filteredFromModTypeIds
             @rst
             The specific :cpp:enum:`ModTypeId`\\s (by id) to accept when classifying #modTypes. If
             this is ``std::nullopt``, every :cpp:enum:`ModTypeId` the classifier reports is
             accepted :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param forcedFromModTypeIds
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
             * @param parseData
             @rst
             Pre-existing parse data for this ``.ini`` file, if some earlier pass already produced it
             -- see #getParseData :raw-html:`<br />` :raw-html:`<br />`

             ``std::nullopt`` (the default) means "not parsed yet", which is **distinct** from a
             present-but-empty map ("parsed, and it found nothing"): #fix re-parses only in the
             former case :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param downloadMode
             @rst
             The download mode used to handle file downloads -- see #downloadMode :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: :cpp:enumerator:`DownloadMode::Normal`
             @endrst
             * @param fromVersion
             @rst
             The game version the ``.ini`` file originates from -- see #fromVersion :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param toVersion
             @rst
             The game version to fix the ``.ini`` file *to* -- see #toVersion :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param filteredToModTypeNames
             @rst
             The names of the target mod types to accept when fixing -- see #filteredToModTypeNames
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            explicit IniFile(std::optional<std::string> file = std::nullopt, std::string txt = "",
                                  std::optional<int> gameTypeId = std::nullopt,
                                  std::optional<std::unordered_set<int>> filteredFromModTypeIds = std::nullopt,
                                  std::optional<std::unordered_set<int>> forcedFromModTypeIds = std::nullopt,
                                  std::optional<std::unordered_map<int, ModType>> overrideModTypes = std::nullopt,
                                  BaseIniClassifier* iniClassifier = nullptr,
                                  std::optional<ParseData> parseData = std::nullopt,
                                  DownloadMode downloadMode = DownloadMode::Normal,
                                  std::optional<Version> fromVersion = std::nullopt,
                                  std::optional<Version> toVersion = std::nullopt,
                                  std::optional<std::unordered_set<std::string>> filteredToModTypeNames = std::nullopt);

            virtual ~IniFile() = default;

            /**
             * @brief
             @rst
             The game version the ``.ini`` file originates from, or ``std::nullopt`` to treat it as
             coming from the latest version :raw-html:`<br />` :raw-html:`<br />`

             This is what #parse hands to :cpp:func:`IniParseBuilder::build`, and so what decides
             *which* parser a version-dependent :cpp:member:`ModType::iniParseBuilder` picks for
             this particular file :raw-html:`<br />` :raw-html:`<br />`

             A plain, publicly mutable member rather than a getter/setter pair, matching both
             #downloadMode and the pure-Python original's own ``self.fromVersion`` :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Reassigning this after a parser has already been built for some mod type does
                **not** rebuild that parser -- #parse caches one parser per mod type (the analogue
                of the original's ``self._iniParser``), and the original has exactly the same
                staleness. Call #clear first if the version needs to change mid-flight

             .. note::
                The pure-Python original also carries a separate ``toVersion`` ("the version to fix
                the .ini file *to*", used by the fixing side rather than the parsing side). That is
                not ported yet -- add it here alongside this when the fixer side gets its own
                builder

             **Default**: ``std::nullopt``
             @endrst
             */
            std::optional<Version> fromVersion;

            /**
             * @brief
             @rst
             The game version to fix the ``.ini`` file **to**, or ``std::nullopt`` for the latest
             :raw-html:`<br />` :raw-html:`<br />`

             The counterpart to #fromVersion. Together they are the two version halves of the
             :cpp:type:`IniFixBuilder::ArgsRepo` key, so this decides *which* fixer a
             version-dependent :cpp:member:`ModType::iniFixBuilder` picks for each target mod
             :raw-html:`<br />` :raw-html:`<br />`

             Mirrors the pure-Python original's own ``self.toVersion``. Publicly mutable, with the
             same staleness caveat as #fromVersion: reassigning it after #fix has already built the
             fixers does not rebuild them -- call #clear first :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            std::optional<Version> toVersion;

            /**
             * @brief
             @rst
             The names of the target mod types to fix to, or ``std::nullopt`` for **all of them**
             :raw-html:`<br />` :raw-html:`<br />`

             One source mod routinely fixes to several targets (``Jean`` fixes to both ``JeanCN``
             and ``JeanSea``), and by default #fix runs the fixer for every target the
             :cpp:type:`IniFixBuilder::ArgsRepo` lists for it. Setting this narrows that fan-out to
             just the named targets :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                ``std::nullopt`` and an **empty set** mean different things: ``std::nullopt`` is
                "no filter, fix to every target", while an empty set filters *everything* out and so
                fixes to nothing

             .. note::
                Mod **names**, not :cpp:enum:`ModTypeId`\\s -- unlike the constructor's
                'filteredFromModTypeIds', which filters the *source* side by id. The fix table is
                keyed by mod name, so filtering by name avoids a lookup back through the registry
                for every candidate :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            std::optional<std::unordered_set<std::string>> filteredToModTypeNames;

            /**
             * @brief
             @rst
             The download mode used to handle file downloads :raw-html:`<br />` :raw-html:`<br />`

             A plain, publicly mutable member rather than a getter/setter pair, matching the
             pure-Python original's own ``self.downloadMode`` -- callers both read it (eg.
             :cpp:func:`RegFillMissing::editFromIni`) and reassign it between passes over the same
             ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

             **Default**: :cpp:enumerator:`DownloadMode::Normal`
             @endrst
             */
            DownloadMode downloadMode = DownloadMode::Normal;

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

             * If the constructor's ``forcedFromModTypeIds`` argument was ``std::nullopt``, calls the
               classifier's own :cpp:func:`BaseIniClassifier::classify` normally: #isMod/#isFixed are
               set from its result, and #modTypes is built from its ``modType`` map (filtered down
               to the constructor's ``filteredFromModTypeIds`` argument, when given), resolving each id
               to an actual :cpp:class:`ModType` via the constructor's ``overrideModTypes``/the
               global registry
             * If the constructor's ``forcedFromModTypeIds`` argument had a value, the classifier is
               never asked to classify a :cpp:enum:`ModTypeId` at all -- only
               :cpp:func:`BaseIniClassifier::checkIsFixedMod` is called, to set #isMod/#isFixed
               independently of any specific mod type, and #modTypes is instead built directly from
               ``forcedFromModTypeIds`` (each id resolved to a :cpp:class:`ModType` the same way),
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
             The result of the last #parse, or ``std::nullopt`` if this ``.ini`` file has never been
             parsed :raw-html:`<br />` :raw-html:`<br />`

             The empty-``optional`` state is meaningful: it is what tells #fix that it still has to
             run the parsers itself. A *present* but empty map means parsing already happened and
             simply produced nothing
             @endrst
             */
            const std::optional<ParseData>& getParseData() const;

            /**
             * @brief
             @rst
             The result of the last #parse -- the mutable overload, so a caller can hand the graphs
             to something that edits them in place
             @endrst
             */
            std::optional<ParseData>& getParseData();

            /**
             * @brief
             @rst
             Clears the text read in from the ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                If #getFile is ``std::nullopt``, the default run of this (with 'eraseSourceTxt'
                ``false``) does **nothing**, because the constructor's ``txt`` is then this object's
                only source of data. Pass ``true`` to wipe that too -- matching the pure-Python
                original's own ``clearRead``
             @endrst
             *
             * @param eraseSourceTxt Whether to also erase the text of a file-less .ini file. **Default**: ``false``
             */
            void clearRead(bool eraseSourceTxt = false);

            /**
             * @brief
             @rst
             Clears all the saved data for the ``.ini`` file -- the read text (via #clearRead), the
             classification results (#isMod / #isFixed / #modTypes / #isClassified), the parsed
             :cpp:class:`IfTemplate`\s, the shared :cpp:class:`Z3Context`, and #getParseData
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The :cpp:class:`Z3Context` is **replaced with a fresh one** rather than cleared in
                place, exactly as the pure-Python original does. The :cpp:class:`IfTemplate`\s are
                dropped first, since they hold predicates belonging to the old context

             .. note::
                The pure-Python original also clears its ``_heading``, ``_resourceBlends`` and the
                resource models (``clearModels``). None of those exist on this class yet -- add them
                here when they land
             @endrst
             *
             * @param eraseSourceTxt Whether to also erase the text of a file-less .ini file -- forwarded to #clearRead. **Default**: ``false``
             */
            void clear(bool eraseSourceTxt = false);

            /**
             * @brief
             @rst
             Parses the ``.ini`` file once per classified :cpp:class:`ModType`, using a parser built
             from each one's own :cpp:member:`ModType::iniParseBuilder` :raw-html:`<br />`
             :raw-html:`<br />`

             Follows the same order as the pure-Python original's own ``parse``:

             #. #classify the file, if it hasn't been classified yet
             #. Bail out if the file was classified as no known mod type at all (the equivalent of
                the original's ``if (self.availableType is None): return``)
             #. Refresh the :cpp:class:`IfTemplate`\\s (see the 'flushIfTemplates' argument)
             #. For each entry of #getModTypes, build its parser from
                :cpp:member:`ModType::iniParseBuilder` -- passing that mod type's
                :cpp:member:`ModType::name` and this file's own #fromVersion, so a version-dependent
                builder picks the parser appropriate to this file -- then
                :cpp:func:`BaseIniParser::clear` and :cpp:func:`BaseIniParser::parse` it. A mod type
                with no builder at all is skipped, the equivalent of the original's ``_getParser``
                returning ``None``

             :raw-html:`<br />`

             .. note::
                The pure-Python original only ever has a *single* ``availableType``, so it parses
                once and returns nothing. A C++ :cpp:class:`IniFile` can be classified as several
                :cpp:class:`ModType`\\s at once (see #modTypes), so this parses once per mod type and
                keys the results by :cpp:enum:`ModTypeId`

             .. note::
                Step 4's built parsers are cached per mod type for the lifetime of this file (until
                #clear), the analogue of the original's ``self._iniParser`` -- a later #fix reuses
                the same parser rather than building a second one. Each is built already bound to
                this file, so unlike the earlier shared-parser design nothing is rebound and two
                :cpp:class:`IniFile`\\s of the same mod type no longer interfere. See
                :cpp:member:`ModType::iniParseBuilder`

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
            ParseData& parse(bool flushIfTemplates = true);

            /**
             * @brief
             @rst
             Fixes the ``.ini`` file once per classified :cpp:class:`ModType`, using a fixer built
             from each one's own :cpp:member:`ModType::iniFixBuilder`, and merges every result
             together :raw-html:`<br />` :raw-html:`<br />`

             For each entry of #getModTypes:

             #. Take that mod type's slice of #getParseData. **If there is none yet, parse it now**
                via a parser built from that mod type's own
                :cpp:member:`ModType::iniParseBuilder`, and cache the result into #getParseData --
                so a later call reuses it rather than re-parsing
             #. Build that mod type's fixer from :cpp:member:`ModType::iniFixBuilder` -- passing its
                :cpp:member:`ModType::name` and this file's own #fromVersion, so a version-dependent
                builder picks the fixer appropriate to this file -- bound to the parser from step 1.
                Skip the mod type if it has no fix builder, or if that parser could not be built:
                the equivalent of the pure-Python original's ``_getFixer`` returning ``None``, which
                likewise refuses to build while ``self._iniParser`` is still ``None``
             #. Call :cpp:func:`BaseIniFixer::fix` with that parse data
             #. Merge the returned file-path/content pairs into the combined result

             :raw-html:`<br />`

             .. note::
                Merging is a plain overwrite: if two mod types both produce content for the *same*
                file path, the one visited later wins. #getModTypes is unordered, so don't rely on
                which that is -- in practice different mod types write different files

             .. note::
                Like #parse's parsers, step 2's built fixers are cached per mod type until #clear,
                the analogue of the original's ``self._iniFixer`` -- a second #fix reuses the same
                fixer rather than building another. Each is built already bound to this file's own
                parser, so nothing is rebound and two :cpp:class:`IniFile`\\s of the same mod type
                do not interfere -- unlike the removers, which are flyweights and *are* shared
                (see #removeFix)

             .. note::
                This classifies the file first if needed, so it can be called without a preceding
                #parse. That is a deliberate divergence from the pure-Python original, whose ``fix``
                assumes the pipeline already ran ``parse``
             @endrst
             *
             * @param keepBackup Whether to keep backups for the .ini file. **Default**: ``true``
             * @param fixOnly Whether to only fix the .ini file without undoing any fixes. **Default**: ``false``
             * @param hideOrig Whether to hide the mod for the original character. **Default**: ``false``
             *
             * @return
             @rst
             The new content of the fixed ``.ini`` file(s), keyed by the file path each one should be
             written to -- empty if nothing was classified, or if no classified mod type could build
             a fixer
             @endrst
             */
            std::unordered_map<std::string, std::string> fix(bool keepBackup = true, bool fixOnly = false, bool hideOrig = false);

            /**
             * @brief
             @rst
             Removes the fix from the ``.ini`` file, once per classified :cpp:class:`ModType`, using
             a remover obtained from each one's own :cpp:member:`ModType::iniRemoveBuilder`
             :raw-html:`<br />` :raw-html:`<br />`

             For each entry of #getModTypes: skip the mod type if it has no
             :cpp:member:`ModType::iniRemoveBuilder`, otherwise call
             :cpp:func:`IniRemoveBuilder::build` with this file -- which hands back a remover already
             bound to it -- and call :cpp:func:`BaseIniRemover::remove` on the result
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike #parse and #fix, this returns a **single string** rather than a map keyed by
                :cpp:enum:`ModTypeId`. Each remover strips its own mod type's fix out of the *same*
                one ``.ini`` file and returns that file's whole new content, so the removers chain:
                the meaningful answer is the content left after the last one ran, not a set of
                independent per-mod-type results

             .. note::
                Also unlike #parse and #fix, the remover is **not** cached on this file. Its builder
                is a flyweight that may hand the same instance to several :cpp:class:`IniFile`\\s and
                only re-points it at the caller on each :cpp:func:`IniRemoveBuilder::build`, so
                asking every time is what keeps the binding correct. This is a deliberate divergence
                from the pure-Python original, which caches into ``self._iniRemover`` and can
                therefore act through a remover another file has since rebound

             .. note::
                :cpp:func:`BaseIniRemover::remove` returns ``""`` in its base (unported) form, so
                this returns ``""`` too once any remover has run. It returns the file's current text
                unchanged when nothing ran at all -- no mod types, or none with a remove builder

             .. note::
                Unlike the pure-Python original's ``_getRemover``, an **unclassified** ``.ini`` file
                does not fall back to :cpp:func:`GlobalIniRemoveBuilders::removeBuilder` here -- it
                simply returns its text unchanged. That fallback exists only as
                :cpp:class:`ModType`'s own null-default. Wire it in here too if the unclassified
                case should really strip fixes, once a concrete C++ remover exists to make that
                meaningful

             .. note::
                Reads the file first if it hasn't been read yet, which is what the pure-Python
                original's ``_readLines`` decorator did for each remover
             @endrst
             *
             * @param parse Whether to also parse for the ``*.RemapBlend.buf`` files that need to be removed. **Default**: ``false``
             * @param writeBack Whether to write back the new text content of the .ini file. **Default**: ``true``
             *
             * @return The new content of the .ini file
             */
            std::string removeFix(bool parse = false, bool writeBack = true);

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

            // There is deliberately no parser_/fixer_ member: both are *built* per file, per mod
            // type, from the ModType's own builders, and this file owns the ones it built -- see
            // builtParsers_/builtFixers_. Removers are the exception -- their builder is a
            // flyweight that owns its own cache, so removeFix() asks it every time rather than
            // caching here (see removeFix's doc comment).
            std::optional<ParseData> parseData_;

            // The parser built for each classified ModType, keyed the same way modTypes/parseData_
            // are -- the analogue of the pure-Python original's own 'self._iniParser', widened to
            // one entry per mod type because a C++ IniFile can be classified as several at once.
            // Populated lazily by getParser(), and dropped by clear().
            //
            // Owned by this file rather than by the ModType, because a ModType is copied by value
            // into modTypes and describes a *kind* of mod: the built parser is bound to this one
            // file, so its lifetime belongs here. The built fixer holds a non-owning pointer into
            // it (BaseIniFixer::getParser), so this must outlive builtFixers_ -- which it does:
            // both are members of this class, both are only emptied by clear(), and clear() drops
            // the fixers first.
            std::unordered_map<int, std::shared_ptr<BaseIniParser<>>> builtParsers_;

            // The fixer built for each classified ModType, keyed identically -- the analogue of the
            // original's 'self._iniFixer'. Populated lazily by getFixer(), dropped by clear().
            // NOTE: a LIST per mod type rather than one fixer, because a source mod can fix to
            // several targets. Each entry is (toModName, fixer). An empty vector is a cached
            // "this mod type contributes nothing", not "not computed yet".
            std::unordered_map<int, std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>>> builtFixers_;

            // Returns the parser for one ModType, building it from that mod type's
            // ModType::iniParseBuilder (passing this file's own 'version') on first use and caching
            // it into builtParsers_ under 'modTypeId' afterwards. nullptr if that mod type has no
            // builder at all -- the equivalent of the original's '_getParser' returning None.
            //
            // 'modTypeId' is the key 'modType' is filed under in modTypes, which is not necessarily
            // ModType::modTypeId -- see the note in the implementation.
            BaseIniParser<>* getParser(int modTypeId, ModType& modType);

            // The fixer counterpart of getParser: builds from ModType::iniFixBuilder, binding the
            // result to that mod type's own built parser. nullptr if the mod type has no fix
            // builder, or if its parser could not be built -- the latter mirroring the original's
            // '_getFixer', which refuses to build while 'self._iniParser' is still None.
            // Returns ONE FIXER PER TARGET MOD, narrowed by filteredToModTypeNames when set.
            // Empty if the mod type has no fix builder, if its parser could not be built, or if the
            // filter excluded every target.
            const std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>>& getFixers(int modTypeId, ModType& modType);

            // Runs one ModType's parser against this file, returning what it produced -- or an
            // empty vector if that mod type has no parser. Shared by parse() and fix(), since fix()
            // parses on demand for any mod type with no cached parse data.
            std::vector<IniGraphGroup<>> parseModType(int modTypeId, ModType& modType);

            std::optional<std::string> file_;
            std::string fileTxt_;
            std::vector<std::string> fileLines_;
            bool fileLinesRead_ = false;

            std::optional<int> gameTypeId_;
            std::optional<std::unordered_set<int>> filteredFromModTypeIds_;
            std::optional<std::unordered_set<int>> forcedFromModTypeIds_;
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
