#ifndef AGRemapCore_RemapService_H
#define AGRemapCore_RemapService_H

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include <tsl/ordered_set.h>

#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/stats/RemapStats.h"
#include "AGRemapCore/view/BaseLogger.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The overall class for remapping mods :raw-html:`<br />` :raw-html:`<br />`

     This is the **model** half of the pure-Python ``RemapService`` (``remapService.py``), merged
     with the parts of the pure-Python ``Mod`` (``model/Mod.py``) that walk a mod folder and drive
     each ``.ini`` file through it. It is deliberately *not* a literal port of either: the
     pure-Python original mixed its UI conversion in with its model logic, and everything on the UI
     side of that line -- turning the user's mod-type *names*, version *strings* and download-mode
     *strings* into real objects, printing the mods that are about to be fixed, deciding where a
     log file goes -- lives in ``RemapServiceCLI`` instead :raw-html:`<br />` :raw-html:`<br />`

     Concretely, what that split buys each attribute:

     * ``types``/``remappedTypes``/``forcedType`` were lists of user-typed mod-type *names* that the
       constructor resolved by string search (and could fail on, with an ``InvalidModType``). Here
       they are #fromModTypeIds, #toModTypeIds and #forcedModTypeIds -- sets of
       :cpp:enum:`ModTypeId` integer values, handed straight to :cpp:class:`IniFile`, which already
       indexes mod types by that same id. They follow :cpp:class:`IniFile`'s own convention for
       those sets exactly, so that handing them over is a plain pass-through: ``std::nullopt``
       means *no filter at all* (every mod type), while a present-but-**empty** set means *accept
       nothing* -- two genuinely different answers, which a bare ``std::unordered_set`` could not
       tell apart
     * ``version`` was a version *string* that the constructor parsed and could reject. Here it is
       #fromVersion, an already-parsed :cpp:class:`Version`
     * ``downloadMode`` was a download-mode *string* that the constructor searched for and could
       reject. Here it is the :cpp:enum:`DownloadMode` enum itself
     * ``defaultType`` is now #defaultModTypeIds -- a *set* of ids rather than a single
       user-typed name, and one this class never second-guesses (the original quietly discarded it
       unless ``readAllInis`` was set, and otherwise defaulted it to Raiden)
     * ``log`` and ``verbose`` are gone outright -- the first is only ever decided from user input,
       and the second is a property of the view (see :cpp:member:`BaseLogger::verbose`), not of the
       remap
     * #gameTypeId is new, and has no pure-Python counterpart on ``RemapService`` at all --
       :cpp:class:`IniFile` needs it to narrow which game's mod types a ``.ini`` file may classify
       as, and only the caller knows which game is being remapped
     * #logger is new for the same reason the above are typed: the pure-Python original built its
       own ``Logger`` internally (from the ``log``/``verbose`` arguments it no longer takes), which
       hard-wired it to the CLI view. Here the view is handed in

     .. note::
        Because every one of those became a *type* rather than a string, none of them can be
        invalid any more, so this class has no counterpart to the pure-Python original's
        ``__errorsBeforeFix`` -- the deferred ``InvalidModType``/``InvalidDownloadMode``/``ValueError``
        it stashed away in the constructor to re-raise at fix time. Rejecting bad user input is
        ``RemapServiceCLI``'s job now, and it happens before a :cpp:class:`RemapService` is ever
        built
     @endrst
     */
    class RemapService {
        public:

            /**
             * @brief Constructs a new remap service
             *
             * @param path
             @rst
             The file location of where to run the fix -- see #path :raw-html:`<br />`
             :raw-html:`<br />`

             If this argument has no value, then will run the fix from wherever the software was
             started (:cpp:func:`FileService::defaultPath`) :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param keepBackups
             @rst
             Whether to keep backup versions of any ``.ini`` files that the fix changes -- see
             #keepBackups :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             *
             * @param fixOnly
             @rst
             Whether to only fix the mods without removing any previous changes the fix may have
             made -- see #fixOnly :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             *
             * @param undoOnly
             @rst
             Whether to only undo the fixes previously made by the fix -- see #undoOnly
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             *
             * @param hideOrig
             @rst
             Whether to not show the mod on the original character -- see #hideOrig
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             *
             * @param readAllInis
             @rst
             Whether to read all the ``.ini`` files that the fix encounters -- see #readAllInis
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike the pure-Python original, this does **not** widen #fromModTypeIds or pick a
                fallback mod type in the constructor. It is stored verbatim and only consulted
                while walking the mod folder

             :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             *
             * @param fromModTypeIds
             @rst
             The ids of the mod types to accept when parsing a ``.ini`` file -- see
             #fromModTypeIds :raw-html:`<br />` :raw-html:`<br />`

             If this argument has no value, then will fix every type of mod the software supports.
             An argument that *has* a value but is empty accepts no mod type at all
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param forcedModTypeIds
             @rst
             The ids of the mod types to forcibly assume for the parsed ``.ini`` files -- see
             #forcedModTypeIds :raw-html:`<br />` :raw-html:`<br />`

             If this argument has no value, then nothing is forced and the ``.ini`` files are
             classified normally :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param defaultModTypeIds
             @rst
             The ids of the mod types to fall back on for a ``.ini`` file the classifier does not
             recognise -- see #defaultModTypeIds :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``{}``, meaning no fallback
             @endrst
             *
             * @param handleExceptions
             @rst
             When an exception is caught, whether to silently stop running the fix -- see
             #handleExceptions :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             *
             * @param fromVersion
             @rst
             The game version the parsed ``.ini`` files originate from -- see #fromVersion
             :raw-html:`<br />` :raw-html:`<br />`

             If this argument has no value, then will retrieve the hashes/indices of the latest
             version :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param toModTypeIds
             @rst
             The ids of the mod types to accept when fixing a ``.ini`` file -- see #toModTypeIds
             :raw-html:`<br />` :raw-html:`<br />`

             If this argument has no value, then will fix the mods at #fromModTypeIds onto all of
             their corresponding remapped mods. An argument that *has* a value but is empty remaps
             onto no mod type at all :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param proxy
             @rst
             The link to the proxy server used for any internet network requests made -- see
             #proxy :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param downloadMode
             @rst
             The download mode to handle file downloads -- see #downloadMode :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: :cpp:enumerator:`DownloadMode::Normal`
             @endrst
             *
             * @param gameTypeId
             @rst
             The id of the game the mods being remapped belong to -- see #gameTypeId
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param logger
             @rst
             The view the fix reports its progress to -- see #logger :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit RemapService(std::optional<std::string> path = std::nullopt,
                                  bool keepBackups = true,
                                  bool fixOnly = false,
                                  bool undoOnly = false,
                                  bool hideOrig = false,
                                  bool readAllInis = false,
                                  std::optional<std::unordered_set<int>> fromModTypeIds = std::nullopt,
                                  std::optional<std::unordered_set<int>> forcedModTypeIds = std::nullopt,
                                  tsl::ordered_set<int> defaultModTypeIds = {},
                                  bool handleExceptions = false,
                                  std::optional<Version> fromVersion = std::nullopt,
                                  std::optional<std::unordered_set<int>> toModTypeIds = std::nullopt,
                                  std::optional<std::string> proxy = std::nullopt,
                                  DownloadMode downloadMode = DownloadMode::Normal,
                                  std::optional<int> gameTypeId = std::nullopt,
                                  std::shared_ptr<BaseLogger> logger = nullptr);

            virtual ~RemapService() = default;

            /**
             * @brief Whether to keep backup versions of any ``.ini`` files that the fix changes
             */
            bool keepBackups;

            /**
             * @brief
             @rst
             Whether to only fix the mods without removing any previous changes the fix may have
             made :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                Setting both this and #undoOnly leaves nothing for the fix to do. The pure-Python
                original raised a ``ConflictingOptions`` for that combination; validating it is
                ``RemapServiceCLI``'s job now
             @endrst
             */
            bool fixOnly;

            /**
             * @brief
             @rst
             Whether to only undo the fixes previously made by the fix -- see #fixOnly's warning
             for the conflicting combination
             @endrst
             */
            bool undoOnly;

            /**
             * @brief Whether to not show the mod on the original character
             */
            bool hideOrig;

            /**
             * @brief
             @rst
             Whether to read all the ``.ini`` files that the fix encounters, rather than only the
             ones whose name marks them as a mod's own ``.ini`` file
             @endrst
             */
            bool readAllInis;

            /**
             * @brief
             @rst
             The ids of the :cpp:enum:`ModTypeId`\s to filter on when parsing a ``.ini`` file
             :raw-html:`<br />` :raw-html:`<br />`

             Feeds :cpp:class:`IniFile`'s own ``filteredFromModTypeIds``, and carries that
             member's meaning exactly: ``std::nullopt`` is *no filter at all* -- every mod type the
             software supports -- while a present-but-empty set accepts no mod type
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Ids are passed through as-is, with no check that each one corresponds to a declared
                :cpp:enum:`ModTypeId` -- matching :cpp:class:`IniFile`, which deliberately indexes
                by plain ``int`` so a custom mod type using an id of its own can still be used
             @endrst
             */
            std::optional<std::unordered_set<int>> fromModTypeIds;

            /**
             * @brief
             @rst
             The ids of the :cpp:enum:`ModTypeId`\s to forcibly assume for the parsed ``.ini``
             files :raw-html:`<br />` :raw-html:`<br />`

             Feeds :cpp:class:`IniFile`'s own ``forcedFromModTypeIds``. When this has a value, the
             ``.ini`` files are never classified at all -- they are simply taken to be of these mod
             types -- so #fromModTypeIds has nothing left to filter :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Where the pure-Python ``forcedType`` was a single mod type, this is a set, so a
                ``.ini`` file can be forced to more than one mod type at once
             @endrst
             */
            std::optional<std::unordered_set<int>> forcedModTypeIds;

            /**
             * @brief
             @rst
             The ids of the :cpp:enum:`ModTypeId`\s to fall back on for a ``.ini`` file the
             classifier does not recognise :raw-html:`<br />` :raw-html:`<br />`

             Feeds :cpp:member:`IniFile::defaultModTypeIds`, and that member's doc comment is where
             the exact rule lives -- in short, these are in play for one situation only: the
             classifier ran and recognised **no** mod type at all. A ``.ini`` file with
             #forcedModTypeIds set never consults the classifier for mod types in the first place,
             and a ``.ini`` file the classifier *did* recognise as a type #fromModTypeIds then
             rejected stays rejected :raw-html:`<br />` :raw-html:`<br />`

             This is what the pure-Python original's ``defaultType`` was for -- except that it was
             a single mod-type *name* the constructor resolved by string search (and could fail
             on), and it silently became ``None`` unless ``readAllInis`` was set. Here it is a set
             of ids the caller states outright, and nothing in this class's constructor overrides
             it :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                An ordered set (``tsl::ordered_set``), not an ``std::unordered_set``: these land in
                :cpp:func:`IniFile::getModTypes`, which is insertion-ordered and which
                :cpp:func:`IniFile::fix` walks to build one fixer per mod type -- so a shuffled
                fallback set would reorder the fix's own output
             @endrst
             */
            tsl::ordered_set<int> defaultModTypeIds;

            /**
             * @brief When an exception is caught, whether to silently stop running the fix
             */
            bool handleExceptions;

            /**
             * @brief
             @rst
             The game version the parsed ``.ini`` files originate from :raw-html:`<br />`
             :raw-html:`<br />`

             Feeds :cpp:member:`IniFile::fromVersion`. If this has no value, then the latest
             version's hashes/indices are used
             @endrst
             */
            std::optional<Version> fromVersion;

            /**
             * @brief
             @rst
             The ids of the :cpp:enum:`ModTypeId`\s to filter on when fixing a ``.ini`` file
             :raw-html:`<br />` :raw-html:`<br />`

             Feeds :cpp:member:`IniFile::filteredToModTypeIds`, and carries the same convention as
             #fromModTypeIds: ``std::nullopt`` remaps each mod at #fromModTypeIds onto *every* mod
             type it corresponds to, while a present-but-empty set remaps onto none
             :raw-html:`<br />` :raw-html:`<br />`

             eg. if #fromModTypeIds is ``{Keqing, Jean}`` and this is ``{JeanSea}``, then this
             class performs the remaps:

             * Keqing --> KeqingOpulent
             * Jean --> JeanSea

             and notably **not** Jean --> JeanCN
             @endrst
             */
            std::optional<std::unordered_set<int>> toModTypeIds;

            /**
             * @brief
             @rst
             The link to the proxy server used for any internet network requests made
             :raw-html:`<br />` :raw-html:`<br />`

             If this has no value, then all internet network requests are assumed not to need to go
             through a proxy server :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The pure-Python original's ``proxy`` setter pushed the new value into a global
                package manager. Nothing here does that -- the proxy is handed to each download at
                the point it happens (see :cpp:func:`RemapIniResource::fix`), so this is a plain
                attribute
             @endrst
             */
            std::optional<std::string> proxy;

            /**
             * @brief The download mode used to handle file downloads
             */
            DownloadMode downloadMode;

            /**
             * @brief
             @rst
             The id of the :cpp:enum:`GameTypeId` for the game the remapped mods belong to
             :raw-html:`<br />` :raw-html:`<br />`

             Feeds :cpp:class:`IniFile`'s own ``gameTypeId``: when it has a value, only mod types
             belonging to that game are candidates while classifying a ``.ini`` file. If it has no
             value, every registered mod type is a candidate, whichever game it came from
             @endrst
             */
            std::optional<int> gameTypeId;

            /**
             * @brief
             @rst
             The view the fix reports its progress to :raw-html:`<br />` :raw-html:`<br />`

             May be ``nullptr``, in which case the fix runs silently -- matching the pure-Python
             ``Mod``/``Model``, whose ``logger`` was likewise optional :raw-html:`<br />`
             :raw-html:`<br />`

             Held by ``std::shared_ptr`` rather than by reference or by value so that a
             :cpp:class:`BaseLogger` subclass defined in `Python`_ stays alive for exactly as long
             as this class needs it, no matter what the `Python`_ side does with its own reference
             @endrst
             */
            std::shared_ptr<BaseLogger> logger;

            /**
             * @brief The statistics gathered about the fix process
             */
            RemapStats stats;

            /**
             * @brief The file path of where the fix is running from
             *
             * @return The file path of where the fix is running from
             */
            const std::string& path() const;

            /**
             * @brief
             @rst
             Sets the file path of where the fix runs from, clearing out any statistics gathered
             for the previous path
             @endrst
             *
             * @param newPath
             @rst
             The new file path to run the fix from, or no value to run it from wherever the
             software was started
             @endrst
             */
            void setPath(std::optional<std::string> newPath);

            /**
             * @brief
             @rst
             Whether the file path the fix runs from is the folder the software was started from
             @endrst
             *
             * @return Whether the fix runs from the folder the software was started from
             */
            bool pathIsCwd() const;

            /**
             * @brief Clears up all the saved data
             *
             * @param clearLog Whether to also clear out any saved data in #logger
             */
            void clear(bool clearLog = true);

            /**
             * @brief
             @rst
             The bookkeeping for #fix's depth-first folder walk :raw-html:`<br />`
             :raw-html:`<br />`

             Passed by reference into #addNeighbourFolders/#addIniNeighbourFolders rather than
             living on :cpp:class:`RemapService` itself, so a #fix leaves no walk state behind on
             the object and two walks can never see each other's :raw-html:`<br />`
             :raw-html:`<br />`

             The three sets are exactly the ones the pure-Python original's ``_fix`` kept as
             locals, under the same names, and they answer three different questions -- a folder
             already **visited** is never processed again, a folder already **visiting** is in the
             queue and must not be queued twice, and a folder already in **gotNeighbours** has
             already had the subtree beneath it enumerated by some earlier folder's recursive scan
             @endrst
             */
            struct FolderWalk {
                /**
                 * @brief The folders still waiting to be visited, most recently added first
                 */
                std::deque<std::string> dirs;

                /**
                 * @brief Every folder the walk has finished with
                 */
                std::unordered_set<std::string> visited;

                /**
                 * @brief Every folder currently sitting in #dirs
                 */
                std::unordered_set<std::string> visiting;

                /**
                 * @brief Every folder already enumerated by some folder's recursive neighbour scan
                 */
                std::unordered_set<std::string> gotNeighbours;

                /**
                 * @brief
                 @rst
                 Queues a folder to be visited, unless it has already been visited or is already
                 queued
                 @endrst
                 *
                 * @param folder The folder to queue
                 */
                void push(const std::string& folder);
            };

            /**
             * @brief
             @rst
             Fixes every mod found from #path :raw-html:`<br />` :raw-html:`<br />`

             Walks folders depth-first, starting at #path. For each folder it reaches:

             #. if the folder holds no ``.ini`` files at all, nothing is handled there -- the walk
                just enumerates what lies beneath it (#addNeighbourFolders) and moves on
             #. otherwise, each of those ``.ini`` files is built into an :cpp:class:`IniFile` (via
                #createIni, so every one of them inherits this class's #fromModTypeIds /
                #forcedModTypeIds / #toModTypeIds / #fromVersion / #gameTypeId / #downloadMode),
                handed to #handleIni, and then asked which folders *it* references
                (#addIniNeighbourFolders) -- **on top of** the same enumeration step, not instead
                of it

             That second half is what lets the walk follow a mod wherever its resources actually
             live, rather than only where the folder tree happens to put them: a ``.ini`` file can
             reference a blend, position, texture or download sitting outside the folder it is in,
             and those folders may themselves hold further mods :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Unlike the pure-Python original's ``_fix``, this raises nothing for the
                ``fixOnly``-and-``undoOnly`` combination (its ``ConflictingOptions``) and prints no
                summary afterwards -- both are ``RemapServiceCLI``'s. This method is only the walk
                and the model work hanging off it
             @endrst
             */
            void fix();

            /**
             * @brief
             @rst
             Whether the run finished with nothing skipped :raw-html:`<br />` :raw-html:`<br />`

             Exactly the two buckets the pure-Python original checks before its ``ENJOY`` banner: a
             skipped ``.ini`` file, or a resource skipped somewhere inside a mod. A resource skipped
             with no mod folder attributed to it deliberately does not count :raw-html:`<br />`
             :raw-html:`<br />`

             Public because :cpp:class:`RemapServiceCLI` gates on it too -- the tips are only worth
             printing after a run that went cleanly -- and two copies of that rule would be one too
             many
             @endrst
             *
             * @return Whether nothing was skipped
             */
            bool noErrors() const;

        protected:

            /**
             * @brief
             @rst
             Handles a single ``.ini`` file the walk has reached -- the work the pure-Python
             original split across ``fixMod``/``fixIni`` :raw-html:`<br />` :raw-html:`<br />`

             In order:

             #. classify the ``.ini`` file
             #. give up on it if it belongs to no mod at all, or if nothing classified and
                #readAllInis is off -- an unrecognised file is only this fix's business when the
                caller said to read everything
             #. unless #fixOnly, remove the previous fix
             #. sort whatever that removal took out into #stats, by each resource's own ``type``
                (see :cpp:func:`RemapStats::get`)
             #. unless #undoOnly, fix the ``.ini`` file, then fix each resource it collected

             :raw-html:`<br />`

             Each resource is screened before being fixed, through the questions
             :cpp:class:`RemapIniResourceMixin` asks -- see #fixResources, which is where that
             sequence and its reasons live
             @endrst
             *
             * @param ini The ``.ini`` file to handle
             */
            virtual void handleIni(IniFile& ini);

            /**
             * @brief
             @rst
             Fixes every resource a ``.ini`` file collected, screening each one first
             :raw-html:`<br />` :raw-html:`<br />`

             The screening is :cpp:class:`RemapIniResourceMixin`'s six questions, asked in the order
             the pure-Python original's ``Mod.handleFixFiles`` asks them, and each one exists
             because the answer is already known without doing any work:

             * ``hasRequired`` -- the resource is missing something it cannot be fixed without (the
               original's missing-``origFullPath`` branch). Recorded as skipped rather than silently
               dropped
             * ``srcIsFixed`` / ``srcEncounteredError`` -- this fix already dealt with the *source*
               file, successfully or not, on some earlier ``.ini`` file. Several ``.ini`` files can
               name the same source
             * ``fixIsFixed`` / ``fixEncounteredError`` -- likewise for the *fixed* file
             * ``fixExists``, but only under #fixOnly -- "fix without removing previous fixes" means
               a fixed file already on disk is left exactly as it is

             :raw-html:`<br />`

             A resource that is not a :cpp:class:`RemapIniResourceMixin` at all is left alone: it
             carries none of these questions and no ``fix`` this class knows how to call
             :raw-html:`<br />` :raw-html:`<br />`

             Screening and fixing are two separate passes, deliberately. The heading this writes
             before the first fix has to know whether **anything** survived screening -- announcing
             that the resources are being fixed and then silently fixing none of them reads worse
             than saying nothing at all. With nothing left, this writes no line and does no work

             .. note::
                Each resource narrates its own fix on top of that one heading (see
                :cpp:member:`IniResource::logger`), so the heading says *that* resources are being
                fixed and the resources themselves say *which*
             @endrst
             *
             * @param ini The ``.ini`` file whose resources to fix
             */
            void fixResources(IniFile& ini);

            /**
             * @brief
             @rst
             Reports what the whole run did, as a ``Summary`` heading of bullet points
             :raw-html:`<br />` :raw-html:`<br />`

             A line only appears when it has something to say: the fixing half is skipped entirely
             under #undoOnly, the removal half under #fixOnly, and within each, a count of zero
             prints nothing rather than "removed 0 files". The two lines that always appear when
             fixing (``.ini`` files and ``Blend.buf`` files) match the pure-Python original, which
             prints those unconditionally because a run that found neither is worth saying out loud
             @endrst
             */
            void reportSummary();

            /**
             * @brief
             @rst
             Reports every ``.ini`` file and resource that was skipped because something went wrong,
             grouped by the mod folder it happened in :raw-html:`<br />` :raw-html:`<br />`

             Written through :cpp:func:`BaseLogger::error` rather than ``log``, as warnings the user
             is meant to notice above the summary
             @endrst
             */
            void reportSkippedMods();

            /**
             * @brief
             @rst
             Queues every folder beneath some folder, so the walk reaches them too
             :raw-html:`<br />` :raw-html:`<br />`

             Enumerates **recursively** -- every descendant folder at any depth, in one pass,
             exactly as the pure-Python original's ``getFilesAndDirs(recursive = True)`` did. A
             folder whose subtree some earlier scan already enumerated
             (:cpp:member:`FolderWalk::gotNeighbours`) is skipped rather than re-walked
             @endrst
             *
             * @param walk The walk to queue the folders into
             * @param folder The folder to enumerate beneath
             */
            void addNeighbourFolders(FolderWalk& walk, const std::string& folder);

            /**
             * @brief
             @rst
             Queues every folder some ``.ini`` file references, so the walk reaches them too
             :raw-html:`<br />` :raw-html:`<br />`

             Those are the folders of the file paths its resource models point at -- for a plain
             :cpp:class:`IniResource` just its ``srcPath``, and for an :cpp:class:`IniFixResource`
             both its ``srcPath`` and its ``fixedPath`` (see
             :cpp:func:`IniFile::getReferencedFolders`, which is where that rule lives) --
             including the ``.ini`` file's downloads :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This only has anything to report once 'ini' has actually been parsed. An
                unparsed :cpp:class:`IniFile` holds no resource models, so calling this before
                #handleIni queues nothing at all
             @endrst
             *
             * @param walk The walk to queue the folders into
             * @param ini The ``.ini`` file to read the referenced folders of
             */
            void addIniNeighbourFolders(FolderWalk& walk, const IniFile& ini);

            /**
             * @brief
             @rst
             Builds the :cpp:class:`IniFile` for some ``.ini`` file path, wiring in every one of
             this class's own remap options :raw-html:`<br />` :raw-html:`<br />`

             The counterpart to the pure-Python original's ``createMod`` (and to ``Mod``'s own
             ``createIniFile``, which is what actually did this wiring) -- collapsed into one step,
             since there is no ``Mod`` in between any more :raw-html:`<br />` :raw-html:`<br />`

             #fromModTypeIds/#forcedModTypeIds/#toModTypeIds are handed over verbatim -- this
             class stores them in :cpp:class:`IniFile`'s own convention precisely so that no
             reinterpretation happens in between :raw-html:`<br />` :raw-html:`<br />`

             #defaultModTypeIds is **assigned** rather than passed: like
             :cpp:member:`IniFile::fromVersion`, it is a public member on that class with no
             constructor parameter of its own
             @endrst
             *
             * @param iniPath The file path of the ``.ini`` file
             *
             * @return The new object representing the ``.ini`` file
             */
            std::unique_ptr<IniFile> createIni(const std::string& iniPath);

            /**
             * @brief
             @rst
             Whether a file is a ``RemapFix`` ``.ini`` **copy** this fix generated -- the third
             bucket of the pure-Python ``Mod::getOptionalFiles``, alongside source ``.ini`` files
             and backups
             @endrst
             *
             * @param file The file path to check
             *
             * @return Whether 'file' is a generated copy
             */
            static bool _isRemapCopyIni(const std::string& file);

            /**
             * @brief
             @rst
             The path of the ``.ini`` file a ``RemapFix`` copy was generated **from** -- the exact
             reverse of the naming :cpp:func:`IniFileFixContext::fixedFilePath` does, and a port of
             ``Mod::getOrigIniPath`` :raw-html:`<br />` :raw-html:`<br />`

             Splits on the **last** occurrence of the suffix, because a mod's own ``.ini`` file is
             free to have it in its name and it is the one this fix appended that has to come off
             @endrst
             *
             * @param remapCopyPath The path of the generated copy
             *
             * @return The path of the ``.ini`` file it came from
             */
            static std::string _origIniPath(const std::string& remapCopyPath);

            /**
             * @brief
             @rst
             Deletes every ``RemapFix`` copy generated from one ``.ini`` file, after that file's own
             fix has been undone :raw-html:`<br />` :raw-html:`<br />`

             Each copy is **undone before it is deleted**, not merely deleted: a copy's fix owns
             resources of its own -- carrying a mod object the source file could not is the whole
             reason it exists -- so removing the file first would strand them
             @endrst
             *
             * @param ini The ``.ini`` file whose copies to remove
             */
            void _removeRemapCopies(const IniFile& ini);

        private:
            std::string path_;
            bool pathIsCwd_ = false;

            // The walk itself. Split out of fix() so that fix() is only the exception/logger
            //   bookkeeping wrapped around it -- the same split the pure-Python original had
            //   between its 'fix' and '_fix'.
            void _fix();

            // Fixes one resource by dispatching on its concrete type -- see _fixResource's own
            //   comment in the .cpp for why there is no virtual fix() to call instead.
            bool _fixResource(IniResource& resource);

            // The path a resource's fix actually writes to: an IniFixResource's fixedPath, or the
            //   srcPath of anything that works in place.
            static std::string _fixedPathOf(const IniResource& resource);

            // Reports the skipped .ini files / resources and the summary, then the footer.
            void report();

            // One "these were skipped" warning block for one kind of resource, grouped by mod.
            void _reportSkippedAsset(const std::string& assetName,
                                     const std::unordered_map<std::string, std::unordered_map<std::string, std::exception_ptr>>& byMods,
                                     const FileStats& resourceStats);

            // One mod folder's worth of skipped resources, as a heading-wrapped bullet list.
            std::string _warnSkippedIniResource(const std::string& modPath, const FileStats& resourceStats) const;

            // The .ini file's base name, for logging -- the logger already carries the folder.
            static std::string _iniName(const IniFile& ini);

            // logger->log, skipped when there is no logger. Every progress message goes through
            //   this rather than repeating the null check.
            void log(const std::string& message);

            // Whether a file is a backup this fix (or an older version of it) left behind. Either
            //   extension: the current one writes a .txt, older ones kept .ini.
            static bool _isBackupIni(const std::string& file);


            // Deletes every backup sitting directly in a folder. Called once per folder, the first
            //   time the walk reaches it, and only when keepBackups is off.
            void _removeBackupInis(const std::string& folder);

            // The direct-child .ini files of a folder that this fix should actually handle, in
            //   the order the OS lists them -- see _isSrcIni for what gets filtered out.
            static std::vector<std::string> _getIniPaths(const std::string& folder);

            // Whether a file is a .ini file that this fix did NOT itself create. Ports the pure-
            //   Python Mod.getOptionalFiles's own first bucket: a .ini file whose name carries any
            //   of this software's markers is one of its own outputs (a backup, or a RemapFix
            //   copy), and handing one back to the fix would be feeding it its own tail.
            static bool _isSrcIni(const std::string& file);

            // Normalizes 'path_' to an absolute path and works out whether it names the folder the
            //   software was started from.
            void _setupModPath(std::optional<std::string> newPath);
    };
}

#endif
