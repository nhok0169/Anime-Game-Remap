#ifndef AGRemapCore_RemapServiceCLI_H
#define AGRemapCore_RemapServiceCLI_H

#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/RemapService.h"
#include "AGRemapCore/view/BaseLogger.h"
#include "AGRemapCore/view/Logger.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The command-line front end for a remap -- the **UI** half of what the pure-Python
     ``RemapService`` used to be :raw-html:`<br />` :raw-html:`<br />`

     :cpp:class:`RemapService` is the model: it takes already-typed data (:cpp:enum:`ModTypeId`
     integers, a parsed :cpp:class:`Version`, a :cpp:enum:`DownloadMode`) and knows nothing about
     where its output goes. This class is everything on the other side of that line -- the view it
     reports through, the log file that output is written to, and (as they are added) turning what a
     user typed into the data the model wants :raw-html:`<br />` :raw-html:`<br />`

     **Holds** a :cpp:class:`RemapService` rather than deriving from one. The model stays completely
     unaware there is a CLI, and #service is public so the caller can reach every one of its
     attributes without this class forwarding them by hand :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Argument parsing is deliberately **not** here. The pure-Python driver (``main.py``) parses
        the command line and hands the results in, and that split is kept: this class takes values,
        not an ``argv``. It is why the option *names* a tip would quote (``--revert`` and friends)
        are not in this class either -- they belong with the parser that defines them

     .. note::
        Written from the pure-Python ``RemapService``'s own ``createLog``/``log`` handling
        (``remapService.py``), which is where the log-file behaviour below comes from
     @endrst
     */
    class RemapServiceCLI {
        public:

            /**
             * @brief Constructs a new command-line front end
             *
             * @param service
             @rst
             The remap this drives -- see #service :raw-html:`<br />` :raw-html:`<br />`

             Taken by value and kept. Its :cpp:member:`RemapService::logger` is **overwritten** with
             this class's own #logger, since reporting is exactly what this class is for
             @endrst
             *
             * @param log
             @rst
             The **folder** to write the log file into, or ``std::nullopt`` not to write one -- see
             #getLog :raw-html:`<br />` :raw-html:`<br />`

             A folder rather than a file path, matching the pure-Python original: the file's own
             name is always :cpp:member:`FileTypes::Log` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param verbose
             @rst
             Whether the fix prints its progress as it runs :raw-html:`<br />` :raw-html:`<br />`

             Independent of 'log': a quiet run can still write a full log file, since the two are
             separate flags on :cpp:class:`BaseLogger` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             */
            explicit RemapServiceCLI(RemapService service, std::optional<std::string> log = std::nullopt,
                                      bool verbose = true);

            /**
             * @brief
             @rst
             Constructs a front end from what a user **typed** -- the string half of the pure-Python
             ``RemapService``'s own constructor :raw-html:`<br />` :raw-html:`<br />`

             Every parameter that :cpp:class:`RemapService` takes as already-typed data
             (:cpp:enum:`ModTypeId` integers, a :cpp:class:`Version`, a :cpp:enum:`DownloadMode`)
             arrives here as the text an argument parser produced, and is converted. Everything
             already unambiguous as a `bool` or a path is passed straight through :raw-html:`<br />`
             :raw-html:`<br />`

             **A bad string does not throw from here.** The first conversion failure is stored and
             raised by #fix instead -- see #hasErrorsBeforeFix for why. So is the pure-Python
             original's own behaviour :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                There is no ``gameType`` **string**: nothing resolves a game type by name (there is
                no :cpp:func:`ModTypeIdTools::findByName` equivalent for one) and no command-line
                option asks for one, so 'gameTypeId' is taken as the id it already is
             @endrst
             *
             * @param path The folder the fix runs from, or ``std::nullopt`` for the current one
             * @param keepBackups Whether the backup files this fix writes are kept. **Default**: ``true``
             * @param fixOnly Whether the fix only fixes, without first undoing. **Default**: ``false``
             * @param undoOnly Whether the fix only undoes, without fixing. **Default**: ``false``
             * @param hideOrig Whether the mod shows only on the remapped character. **Default**: ``false``
             * @param readAllInis Whether unclassified ``.ini`` files are read too. **Default**: ``false``
             * @param types
             @rst
             The names/aliases of the mod types to fix **from**, or ``std::nullopt`` / an empty list
             for every one of them :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                An empty list means *no filter* here, exactly as ``std::nullopt`` does -- it is the
                pure-Python original's own reading, and the only one an argument parser can produce
                (a user who names no types wants all of them, not none). That is why this differs
                from :cpp:member:`RemapService::fromModTypeIds`, where an empty **set** is a filter
                that accepts nothing: by then the ambiguity has been resolved, and this is where it
                is resolved

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param defaultType
             @rst
             The name of the mod type to assume for an ``.ini`` file nothing could classify
             :raw-html:`<br />` :raw-html:`<br />`

             Only ever in play when 'readAllInis' is on and no 'forcedType' was given -- otherwise
             there is either nothing unclassified to fall back for, or a forced type already
             answering the question. Left unset with 'readAllInis' on, it is ``Raiden``, which is
             the pure-Python original's own default :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param forcedType
             @rst
             The name of the mod type to assume for **every** ``.ini`` file, classified or not
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param log The **folder** to write the log file into, or ``std::nullopt`` for none. **Default**: ``std::nullopt``
             * @param verbose Whether the fix prints its progress as it runs. **Default**: ``true``
             * @param handleExceptions Whether a failure is logged rather than thrown. **Default**: ``false``
             * @param version
             @rst
             The game version to fix to, as a `PEP 440`_ string, or ``std::nullopt`` for the latest
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param remappedTypes
             @rst
             The names/aliases of the mod types to fix **to**, or ``std::nullopt`` / an empty list
             for every one of them -- see 'types' for why an empty list is not an empty filter
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param proxy The proxy used for file downloads. **Default**: ``std::nullopt``
             * @param downloadMode
             @rst
             The name of how file downloads are handled -- ``"disabled"``, ``"normal"`` or
             ``"always"``, ignoring case and surrounding whitespace :raw-html:`<br />`
             :raw-html:`<br />`

             Unset means :cpp:enumerator:`DownloadMode::Normal`. The pure-Python original named a
             ``HardTexDriven`` mode here that no longer exists, so its own unset case has been
             broken for as long as that member has been gone :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @param gameTypeId The game to fix for, as an id. **Default**: ``std::nullopt``
             */
            explicit RemapServiceCLI(std::optional<std::string> path,
                                      bool keepBackups = true,
                                      bool fixOnly = false,
                                      bool undoOnly = false,
                                      bool hideOrig = false,
                                      bool readAllInis = false,
                                      std::optional<std::vector<std::string>> types = std::nullopt,
                                      std::optional<std::string> defaultType = std::nullopt,
                                      std::optional<std::string> forcedType = std::nullopt,
                                      std::optional<std::string> log = std::nullopt,
                                      bool verbose = true,
                                      bool handleExceptions = false,
                                      std::optional<std::string> version = std::nullopt,
                                      std::optional<std::vector<std::string>> remappedTypes = std::nullopt,
                                      std::optional<std::string> proxy = std::nullopt,
                                      std::optional<std::string> downloadMode = std::nullopt,
                                      std::optional<int> gameTypeId = std::nullopt);

            // Subclassable from outside core (the CLI layer that owns the argument parser
            //   overrides addTips), so destruction has to go through the vtable.
            virtual ~RemapServiceCLI() = default;

            /**
             * @brief
             @rst
             The remap this drives :raw-html:`<br />` :raw-html:`<br />`

             Public on purpose: every model-side option lives on it, and forwarding fifteen
             attributes through this class would be duplication with nothing to gain
             @endrst
             */
            RemapService service;

            /**
             * @brief
             @rst
             The view everything is reported through, shared with #service :raw-html:`<br />`
             :raw-html:`<br />`

             Its ``logTxt`` is set from whether a log folder was given, which is what makes
             :cpp:func:`BaseLogger::loggedTxt` accumulate the text #createLog later writes out. With
             no log folder, nothing is accumulated at all rather than buffered and thrown away
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Declared as the **base** :cpp:class:`BaseLogger` even though the constructor builds a
                concrete :cpp:class:`Logger`. Nothing here needs more than the base's interface, and
                the wider type is what lets a caller substitute a view of their own -- including one
                written in `Python`_, which matters because :cpp:class:`Logger` itself is not a
                `pybind11`_-registered type (the `Python`_-facing ``Logger`` is a separate class, not
                a binding of this one), so a ``shared_ptr`` to it could not cross that boundary at
                all
             @endrst
             */
            std::shared_ptr<BaseLogger> logger;

            /**
             * @brief
             @rst
             The full path of the log file, or ``std::nullopt`` when no log is being written
             @endrst
             *
             * @return The log file's path, if there is one
             */
            /**
             * @brief
             @rst
             Whether a string handed to the string constructor could not be converted
             :raw-html:`<br />` :raw-html:`<br />`

             A conversion failure is **stored rather than thrown**, and #fix raises it. Three
             reasons, all the pure-Python original's: a half-built object is still a usable one to
             inspect, a caller that never runs the fix never had a problem, and #printModsToFix has
             nothing truthful to print about mod types that could not be resolved -- so it is
             skipped entirely rather than printing a half-list :raw-html:`<br />` :raw-html:`<br />`

             Only the **first** failure is kept. The later setups still run, but a second failure is
             dropped, so what surfaces is the first thing wrong rather than the last
             @endrst
             *
             * @return Whether the conversion failed
             */
            bool hasErrorsBeforeFix() const;

            /**
             * @brief
             @rst
             Rethrows the stored conversion failure, or does nothing when there was none
             @endrst
             */
            void raiseErrorsBeforeFix() const;

            const std::optional<std::string>& getLog() const;

            /**
             * @brief
             @rst
             Sets the folder the log file is written into, or ``std::nullopt`` to stop writing one
             :raw-html:`<br />` :raw-html:`<br />`

             Also updates #logger's ``logTxt``, so that turning logging on part-way through starts
             accumulating from that point rather than silently writing an empty file
             @endrst
             *
             * @param newLog The folder to write the log into, or ``std::nullopt`` for none
             */
            void setLog(std::optional<std::string> newLog);

            /**
             * @brief
             @rst
             Whether the fix prints its progress as it runs :raw-html:`<br />` :raw-html:`<br />`

             Reads straight off #logger rather than out of a copy kept here. The pure-Python
             original kept both and only ever wrote the copy through its own setter, so assigning
             ``logger.verbose`` directly left the two disagreeing with no way to tell which was
             being believed. There is one answer now, and it is the view's :raw-html:`<br />`
             :raw-html:`<br />`

             Independent of whether a log file is being written -- see #setLog. A quiet run still
             accumulates a full log
             @endrst
             *
             * @return Whether progress is printed, or ``false`` when there is no view to print
             *   through at all
             */
            bool getVerbose() const;

            /**
             * @brief
             * Sets whether the fix prints its progress as it runs
             *
             * @param newVerbose Whether progress is printed
             */
            void setVerbose(bool newVerbose);

            /**
             * @brief
             @rst
             Runs the remap, then writes the log file :raw-html:`<br />` :raw-html:`<br />`

             The log is written **whatever** happened, including when
             :cpp:member:`RemapService::handleExceptions` is off and the fix throws -- a run that
             failed is the one whose log is worth keeping. The exception still propagates
             @endrst
             */
            virtual void fix();

            /**
             * @brief
             @rst
             Writes everything reported so far out to the log file :raw-html:`<br />`
             :raw-html:`<br />`

             Does nothing when no log folder was given. Announces itself first, so the line naming
             the log file is itself in the log
             @endrst
             */
            virtual void createLog();

            /**
             * @brief
             @rst
             Prints whatever the user might find useful to know next -- **nothing at all** in this
             class :raw-html:`<br />` :raw-html:`<br />`

             A tip is a sentence like "run this script again using the ``--undo`` option", and those
             option names belong to the argument parser, which deliberately is not here (see this
             class's own note). Core has no business inventing them, so this is an empty hook and
             the layer that owns the parser overrides it :raw-html:`<br />` :raw-html:`<br />`

             Called by #fix after the remap and **before** #createLog, so the tips land in the log
             file too, and only when :cpp:func:`RemapService::noErrors` -- advice about what to try
             next is noise on top of a run that already went wrong. That ordering and that gate are
             the pure-Python original's own
             @endrst
             */
            virtual void addTips();

            /**
             * @brief
             @rst
             Prints the banner naming which types of mods this run will fix :raw-html:`<br />`
             :raw-html:`<br />`

             Reads :cpp:member:`RemapService::fromModTypeIds` and prints one bullet per type, sorted
             by NAME rather than by id, so the list reads alphabetically instead of in whatever order
             the ids happen to fall in :raw-html:`<br />` :raw-html:`<br />`

             The two empty cases are **not** the same thing, and this is the one place a user sees
             the difference: ``std::nullopt`` is "no filter" and prints *All mods*, while a
             present-but-empty set accepts nothing and prints *No mods*. The pure-Python original had
             only one empty (a falsy list) and printed *All mods* for it :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Called at the top of #fix, where the pure-Python original called it at the end of its
                **constructor**. Deliberate: a virtual call from a constructor does not reach an
                override, so a subclass reshaping this banner -- the whole reason it is virtual --
                would silently never be used. Running it from #fix costs nothing else, since the
                driver constructs and then immediately fixes
             @endrst
             */
            virtual void printModsToFix();

        private:
            std::optional<std::string> log_;

            // The first conversion failure from the string constructor, raised by fix(). Null when
            //   there was none, which is every object built from the RemapService constructor.
            std::exception_ptr errorsBeforeFix_;

            // The display name for a mod type id, for the banner.
            static std::string _modTypeName(int modTypeId);

            // Turns the folder given to setLog into the full file path written to, or leaves it
            //   empty. The file's own name is always FileTypes::Log.
            void _setupLogPath(std::optional<std::string> newLog);

            // Keeps 'error' only if nothing has failed yet -- the FIRST failure is the one that
            //   surfaces, matching the pure-Python original's own '__errorsBeforeFix is None' guard
            //   on every one of its setups.
            void _recordError(std::exception_ptr error);

            // A mod type id for one name/alias. Records InvalidModType and returns nullopt when
            //   nothing matches.
            std::optional<int> _toModTypeId(const std::string& name);

            // A set of mod type ids for a list of names/aliases, or nullopt for "no filter" when
            //   the list is absent OR empty -- see the 'types' parameter for why those are the
            //   same thing here and not on RemapService.
            std::optional<std::unordered_set<int>> _toModTypeIds(const std::optional<std::vector<std::string>>& names);

            // The string -> model conversions, in the pure-Python original's own order (the forced
            //   type first, since the two after it read whether one was given).
            void _setupForcedModType(const std::optional<std::string>& forcedType);
            void _setupDefaultModType(const std::optional<std::string>& defaultType, const std::optional<std::string>& forcedType);
            void _setupToFixModTypes(const std::optional<std::vector<std::string>>& types, const std::optional<std::string>& forcedType);
            void _setupRemappedTypes(const std::optional<std::vector<std::string>>& remappedTypes);
            void _setupVersion(const std::optional<std::string>& version);
            void _setupDownloadMode(const std::optional<std::string>& downloadMode);
    };
}

#endif
