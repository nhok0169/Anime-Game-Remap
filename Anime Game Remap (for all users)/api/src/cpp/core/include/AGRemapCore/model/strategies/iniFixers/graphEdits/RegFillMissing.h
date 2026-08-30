#ifndef AGRemapCore_RegFillMissing_H
#define AGRemapCore_RegFillMissing_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/RegFillMissingMode.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphEdit`

     Fills the :cpp:class:`IfContentPart`\\s of some caller/callee graph that are missing a
     particular register
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegFillMissing: public BaseIniGraphEdit<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseIniGraphEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniGraphEdit::Graph
             */
            using Graph = typename Base::Graph;

            /**
             * @copydoc BaseIniGraphEdit::PartFilter
             */
            using PartFilter = typename Base::PartFilter;

            /**
             * @copydoc BaseIniGraphEdit::IterData
             */
            using IterData = typename Base::IterData;

            /**
             * @copydoc BaseIniGraphEdit::OrderRanges
             */
            using OrderRanges = typename Base::OrderRanges;

            /**
             * @brief The `sections`_ this edit walks over
             */
            using Section = typename Graph::Section;

            /**
             * @brief The parts this edit fills
             */
            using ContentPart = typename Graph::ContentPart;

            /**
             * @brief
             @rst
             The running `KVP`_ state a \ref PartSelection::partFilter can inspect, when
             \ref trackKeys is on
             @endrst
             */
            using Colouring = typename Graph::Colouring;

            /**
             * @copydoc BaseIniGraphEdit::KeySet
             */
            using KeySet = typename Base::KeySet;

            /**
             * @brief
             @rst
             How to fill one :cpp:class:`IfContentPart` that is missing \ref reg :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Whether new `KVPs`_ land at the front or the back of a part is **already baked into
                the function** by the time one of these exists -- see \ref makeFillMissing's
                ``toFront``. The pure-Python original decided that per edit, from
                :cpp:member:`fillMode`, inside its own ``_getFillMissingFunc``; a caller who
                reassigns \ref fillMode after construction has to rebuild \ref fillMissing to match
                (which is exactly what the `pybind11`_ layer does, on every single ``edit`` call)
             @endrst
             */
            using FillMissingFunc = std::function<void(ContentPart&)>;

            /**
             * @brief
             @rst
             Which parts a fill is allowed to touch, and the state a filter gets to decide that from
             :raw-html:`<br />` :raw-html:`<br />`

             A default-constructed instance (an empty \ref partFilter, no tracking) accepts every
             part, which is what makes the whole selection feature opt-in: \ref fillMissingGraph and
             \ref addCover then behave exactly as they did before it existed, and skip the graph walk
             entirely
             @endrst
             */
            struct PartSelection {
                /**
                 * @brief
                 @rst
                 Decides whether one part may be filled: an **empty** :cpp:class:`Ranges` result
                 skips that part, any non-empty result accepts it :raw-html:`<br />`
                 :raw-html:`<br />`

                 This is the same convention -- and the same
                 :cpp:type:`BaseIniGraphEdit::PartFilter` type -- that
                 :cpp:class:`GraphGroupEdit` already applies to its register edits
                 (``if (keyRanges.isEmpty()) continue;``). Only which *parts* get filled is
                 decided here; a non-empty result's actual ranges are **not** consulted, since
                 filling a part appends a whole `KVP`_ rather than editing occurrences at
                 particular order indices :raw-html:`<br />` :raw-html:`<br />`

                 An empty ``std::function`` accepts every part
                 @endrst
                 */
                PartFilter partFilter;

                /**
                 * @brief The type of mod to fix, handed straight to \ref partFilter. **Nullable**
                 */
                const ModType* modType = nullptr;

                /**
                 * @brief The associated .ini file, handed straight to \ref partFilter. **Nullable**
                 */
                IniFile* ini = nullptr;

                /**
                 * @brief
                 @rst
                 Whether to give \ref partFilter a populated
                 :cpp:member:`SectionIterData::colouring` -- the running state of the `KVPs`_ seen
                 on the path reaching each part :raw-html:`<br />` :raw-html:`<br />`

                 When ``false``, ``colouring`` is ``nullptr`` and a filter can only discriminate on
                 the part/`section`_ itself
                 @endrst
                 */
                bool trackKeys = false;

                /**
                 * @brief
                 @rst
                 Which keys \ref trackKeys should colour, or ``std::nullopt`` for **every** key --
                 the same convention :cpp:func:`IfContentPartColouring::updateColouring` and
                 :cpp:class:`GraphGroupEdit` already use
                 @endrst
                 */
                std::optional<KeySet> keysToTrack;
            };

            /**
             * @brief The register to search for
             */
            K reg;

            /**
             * @brief
             @rst
             How to fill :cpp:class:`IfContentPart`\\s that are missing \ref reg -- see
             #FillMissingFunc. An empty function makes every mode a no-op
             @endrst
             */
            FillMissingFunc fillMissing;

            /**
             * @brief
             @rst
             What mode is used to search for and fill the missing register :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: :cpp:enumerator:`RegFillMissingMode::FillMissing`
             @endrst
             */
            RegFillMissingMode fillMode = RegFillMissingMode::FillMissing;

            /**
             * @brief
             @rst
             Whether the editing is dependent on :cpp:member:`IniFile::downloadMode` -- see
             \ref editFromIni :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             */
            bool dependOnDownload = false;

            /**
             * @brief
             @rst
             Whether to track the `KVPs`_ seen so far for colouring while walking the graph, so the
             ``partFilter`` handed to \ref edit gets a populated
             :cpp:member:`SectionIterData::colouring` to decide from -- see
             \ref PartSelection::trackKeys :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This is **combined with**, not replaced by, the ``trackKeys`` \ref edit is handed by
                its caller (:cpp:class:`GraphGroupEdit` passes its own down). Either side asking for
                tracking is enough -- see \ref effectiveTrackKeys
             @endrst
             *
             * **Default**: ``false``
             */
            bool trackKeys = false;

            /**
             * @brief
             @rst
             Which keys \ref trackKeys should colour, or ``std::nullopt`` for **every** key -- see
             \ref PartSelection::keysToTrack :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                When this is set it **overrides** the ``keysToTrack`` \ref edit is handed by its
                caller; when it is ``std::nullopt`` the caller's is inherited instead -- see
                \ref effectiveKeysToTrack
             @endrst
             *
             * **Default**: ``std::nullopt``
             */
            std::optional<KeySet> keysToTrack;

            /**
             * @brief Constructs a new missing-register-filling edit
             *
             * @param reg The register to search for
             * @param fillMissing How to fill the parts missing 'reg' -- see #FillMissingFunc
             * @param fillMode What mode used to search and fill the missing register. **Default**: ``RegFillMissingMode::FillMissing``
             * @param dependOnDownload Whether the editing is dependent on the .ini file's download mode. **Default**: ``false``
             * @param trackKeys Whether to track `KVPs`_ for colouring while walking the graph. **Default**: ``false``
             * @param keysToTrack Which keys to track, or ``std::nullopt`` for all of them. **Default**: ``std::nullopt``
             */
            explicit RegFillMissing(K reg = K(), FillMissingFunc fillMissing = {},
                                     RegFillMissingMode fillMode = RegFillMissingMode::FillMissing,
                                     bool dependOnDownload = false, bool trackKeys = false,
                                     std::optional<KeySet> keysToTrack = std::nullopt);

            /**
             * @brief
             @rst
             Builds a #FillMissingFunc adding the single `KVP`_ ``reg = value`` to a part
             @endrst
             *
             * @param reg The register to add
             * @param value The value to add for 'reg'
             * @param toFront Whether the `KVP`_ is added to the front of the part instead of the back. **Default**: ``false``
             *
             * @return The filler
             */
            static FillMissingFunc makeFillMissing(K reg, V value, bool toFront = false);

            /**
             * @brief
             @rst
             Builds a #FillMissingFunc adding every `KVP`_ in 'kvps' to a part
             @endrst
             *
             * @param kvps The `KVPs`_ to add
             * @param toFront Whether the `KVPs`_ are added to the front of the part instead of the back. **Default**: ``false``
             *
             * @return The filler
             */
            static FillMissingFunc makeFillMissing(std::vector<std::pair<K, V>> kvps, bool toFront = false);

            /**
             * @brief
             @rst
             Combines this edit's own \ref trackKeys with the one its caller handed down
             :raw-html:`<br />` :raw-html:`<br />`

             Either side asking for tracking is enough (a plain ``||``). A populated colouring is
             strictly more information for a ``partFilter`` than none, and this edit's own
             ``trackKeys`` is a plain ``bool`` whose ``false`` cannot be told apart from "not set" --
             so there is deliberately no way to opt *out* of a caller's tracking. Turning it off is
             only ever a performance choice; it never changes which parts get filled
             @endrst
             *
             * @param callerTrackKeys The key-tracking default handed down by the caller
             *
             * @return Whether to track keys for this edit
             */
            bool effectiveTrackKeys(bool callerTrackKeys) const;

            /**
             * @brief
             @rst
             Picks between this edit's own \ref keysToTrack and the one its caller handed down
             :raw-html:`<br />` :raw-html:`<br />`

             This edit's own set wins when it has one; otherwise the caller's is inherited. Note that
             ``std::nullopt`` on **both** sides still means "every key" once tracking is on -- this
             resolves *which* set to use, not whether the result is narrowing
             @endrst
             *
             * @param callerKeysToTrack The key-tracking key set handed down by the caller
             *
             * @return The key set to colour with
             */
            const std::optional<KeySet>& effectiveKeysToTrack(const std::optional<KeySet>& callerKeysToTrack) const;

            /**
             * @brief
             @rst
             Fills the :cpp:class:`IfContentPart`\\s from 'graph' that are missing 'reg'
             :raw-html:`<br />` :raw-html:`<br />`

             A part reachable from more than one `section`_ is filled exactly once :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                'selection' narrows *which* of the missing parts actually get filled. A
                default-constructed 'selection' accepts all of them and skips the graph walk
                entirely, so the no-selection case costs exactly what it did before this parameter
                existed
             @endrst
             *
             * @param graph The graph to search, modified in place
             * @param reg The register to search
             * @param fillMissing The function to modify the parts that are missing the desired register
             * @param selection Which of the missing parts may be filled. **Default**: accept every part
             *
             * @return The same graph that was passed in, with its missing parts filled
             */
            static Graph& fillMissingGraph(Graph& graph, const K& reg, const FillMissingFunc& fillMissing,
                                            const PartSelection& selection = {});

            /**
             * @brief
             @rst
             Fills a fresh top :cpp:class:`IfContentPart` at each of 'graph''s roots, if 'reg' is
             missing in some :cpp:class:`IfContentPart` of 'graph' :raw-html:`<br />`
             :raw-html:`<br />`

             Nothing is added at all when every root already fully covers 'reg' :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                'selection' narrows *which roots* get covered. Its ``partFilter`` is asked once per
                root, against that root's own first :cpp:class:`IfContentPart` (the part
                :cpp:func:`IfTemplate::addTopContentPart` would reuse or insert before) -- a root
                with no :cpp:class:`IfContentPart` at all is accepted, there being nothing to
                discriminate on. The colouring handed over is **empty by construction**: nothing
                precedes a root, so ``sectionName``/``section`` are the useful discriminators here,
                not the tracked `KVPs`_
             @endrst
             *
             * @param graph The graph to search, modified in place
             * @param reg The register to search
             * @param fillMissing The function to modify the parts that are missing the desired register
             * @param selection Which roots may be covered. **Default**: accept every root
             *
             * @return The same graph that was passed in, with its roots covered
             */
            static Graph& addCover(Graph& graph, const K& reg, const FillMissingFunc& fillMissing,
                                    const PartSelection& selection = {});

            /**
             * @brief
             @rst
             Fills the parts of 'graph' missing \ref reg, honouring
             :cpp:member:`IniFile::downloadMode` when \ref dependOnDownload is set :raw-html:`<br />`
             :raw-html:`<br />`

             With \ref dependOnDownload unset this is just \ref edit. Otherwise
             :cpp:enumerator:`DownloadMode::Disabled` skips the edit entirely, and
             :cpp:enumerator:`DownloadMode::Always` normalizes the graph's branching structure
             first, so that a part missing the register on *some* branch is guaranteed to be its own
             :cpp:class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

             A ``nullptr`` 'ini' reads as :cpp:enumerator:`DownloadMode::Normal` -- the mode that
             adds no download-specific behaviour of its own :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike the pure-Python original -- which called ``self.edit(graph, modType,
                modName = modName)`` with no ``partFilter`` at all -- this **forwards** 'partFilter'
                to \ref edit. Dropping it would silently disable the whole part-selection feature
                for exactly the callers that supply a filter: :cpp:class:`GraphGroupEdit` routes
                through ``editFromIni`` (not ``edit``) whenever it has an ``.ini`` file
             @endrst
             *
             * @param graph The graph to edit, modified in place
             * @param ini The associated .ini file, read for its own download mode, and handed to 'partFilter'. **Nullable**
             * @param modType The type of mod to fix. **Nullable**. Only handed to 'partFilter'
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             * @param partFilter Which parts may be filled -- empty accepts every part. **Default**: empty
             * @param trackKeys The caller's key-tracking default -- combined with \ref trackKeys. **Default**: ``false``
             * @param keysToTrack The caller's key-tracking key set -- overridden by \ref keysToTrack when set. **Default**: ``std::nullopt``
             *
             * @return The same graph that was passed in, after editing
             */
            Graph& editFromIni(Graph& graph, IniFile* ini, const ModType* modType,
                                const std::string& modName = "", const PartFilter& partFilter = {},
                                bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt) override;

            /**
             * @brief
             @rst
             Fills the parts of 'graph' missing \ref reg, by whichever strategy \ref fillMode names
             -- \ref fillMissingGraph for :cpp:enumerator:`RegFillMissingMode::FillMissing`,
             \ref addCover for :cpp:enumerator:`RegFillMissingMode::TopdownCover` :raw-html:`<br />`
             :raw-html:`<br />`

             'partFilter' restricts *which* parts (or, under
             :cpp:enumerator:`RegFillMissingMode::TopdownCover`, which roots) are filled -- an empty
             :cpp:class:`Ranges` result skips that one. Together with \ref trackKeys /
             \ref keysToTrack it forms the \ref PartSelection this hands down :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The pure-Python original accepted 'partFilter' and dropped it, so this edit applied
                to every missing part unconditionally. Honouring it is a deliberate behaviour
                change; an omitted/empty filter still fills everything, exactly as before
             @endrst
             *
             * @param graph The graph to edit, modified in place
             * @param modType The type of mod to fix -- not read here, only handed to 'partFilter'
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             * @param partFilter Which parts may be filled -- empty accepts every part. **Default**: empty
             * @param trackKeys The caller's key-tracking default -- combined with \ref trackKeys via \ref effectiveTrackKeys. **Default**: ``false``
             * @param keysToTrack The caller's key-tracking key set -- resolved by \ref effectiveKeysToTrack. **Default**: ``std::nullopt``
             *
             * @return The same graph that was passed in, after editing
             */
            Graph& edit(Graph& graph, const ModType* modType,
                         const std::string& modName = "", const PartFilter& partFilter = {},
                         bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt) override;

        private:

            /**
             * @brief
             @rst
             The shared body of \ref edit and \ref editFromIni :raw-html:`<br />` :raw-html:`<br />`

             Exists so a ``partFilter`` reached through \ref editFromIni is handed the **real**
             ``.ini`` file rather than ``nullptr``: \ref edit's own signature (inherited from
             :cpp:class:`BaseIniGraphEdit`) has nowhere to carry one, so routing both entry points
             through here is what lets the ``.ini``-aware caller keep it
             @endrst
             *
             * @param graph The graph to edit, modified in place
             * @param ini The associated .ini file, handed to 'partFilter'. **Nullable**
             * @param modType The type of mod to fix, handed to 'partFilter'. **Nullable**
             * @param modName The name of the mod to fix to. Unused
             * @param partFilter Which parts may be filled -- empty accepts every part
             * @param trackKeys The caller's key-tracking default, combined with this edit's own via \ref effectiveTrackKeys
             * @param keysToTrack The caller's key-tracking key set, resolved against this edit's own via \ref effectiveKeysToTrack
             *
             * @return The same graph that was passed in, after editing
             */
            Graph& editImpl(Graph& graph, IniFile* ini, const ModType* modType,
                             const std::string& modName, const PartFilter& partFilter,
                             bool trackKeys, const std::optional<KeySet>& keysToTrack);
    };
}

#include "RegFillMissing.tpp"

#endif
