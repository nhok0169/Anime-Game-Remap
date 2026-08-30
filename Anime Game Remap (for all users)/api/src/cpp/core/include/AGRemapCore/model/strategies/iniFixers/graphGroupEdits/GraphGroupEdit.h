#ifndef AGRemapCore_GraphGroupEdit_H
#define AGRemapCore_GraphGroupEdit_H

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Edits the individual :cpp:class:`IniSectionGraph`\\s from a group of graphs :raw-html:`<br />`
     :raw-html:`<br />`

     Each graph is handed a list of edits, which are applied in order. Consecutive edits *of the
     same kind* are applied as one group -- a run of graph-level edits runs straight over the whole
     graph, while a run of register-level edits runs once per :cpp:class:`IfContentPart` the graph
     iterates over (so a single pass over the graph covers the whole run, rather than one pass per
     edit) :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The individual edits are reached through #PartEdit rather than through
        :cpp:class:`BaseIniGraphEdit`/:cpp:class:`BaseRegEdit` pointers, because the two kinds do
        not share a callable base: their ``edit`` signatures genuinely differ (a whole graph vs. one
        part). See #PartEdit's own note
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphGroupEdit: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseIniGraphGroupEdit::GraphGroups
             */
            using GraphGroups = typename Base::GraphGroups;

            /**
             * @copydoc BaseIniGraphGroupEdit::Graph
             */
            using Graph = typename Base::Graph;

            /**
             * @copydoc IniGraphGroup::ModObj
             */
            using ModObj = typename Base::ModObj;

            /**
             * @brief The type of part a register-level edit edits
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The per-part iteration data a #PartFilter is handed
             */
            using IterData = SectionIterData<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ranges of valid `KVP`_ order indices a #PartFilter returns
             */
            using OrderRanges = Ranges<long long>;

            /**
             * @brief The set of `KVP`_ keys to track while iterating, when key tracking is on
             */
            using KeySet = std::unordered_set<K, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             Restricts an edit to specific areas of a :cpp:class:`IfContentPart` -- an empty
             ``std::function`` means "the whole part", the equivalent of the pure-Python original's
             own ``Ranges.createFull()`` default filter
             @endrst
             */
            using PartFilter = std::function<OrderRanges(const IterData&, const ModType*, IniFile*)>;

            /**
             * @brief Which of the two kinds of edit a #PartEdit is
             */
            enum class PartEditKind {
                /**
                 * @brief Not an edit this class knows how to apply -- skipped
                 */
                None,

                /**
                 * @brief Edits a whole graph at once (the ``graphEdits/`` family)
                 */
                GraphEdit,

                /**
                 * @brief Edits one :cpp:class:`IfContentPart` at a time (the ``regEdits/`` family)
                 */
                RegEdit
            };

            /**
             * @brief
             @rst
             One edit in a graph's edit list :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This exists instead of holding :cpp:class:`BaseIniGraphEdit`/:cpp:class:`BaseRegEdit`
                pointers directly. Two reasons, and the second is the load-bearing one:

                #. The two kinds have no common base with a callable ``edit`` -- their signatures
                   genuinely differ (a whole graph vs. one part), which is exactly why
                   :cpp:class:`BaseIniPartEdit` declares no ``edit`` at all
                #. On the `Python`_ side these lists can be **heterogeneous in implementation
                   language**: most entries are C++-backed (``RegAdd``, ``GraphRename``,
                   ``RegFillMissing`` and friends), but a graph edit is still free to be a pure
                   `Python`_ subclass overriding only ``edit`` (``RegSurroundedAdd`` is exactly
                   that). A C++ pointer to the base cannot reach such an override without a
                   trampoline, so the *dispatch* has to be the abstraction
             @endrst
             */
            class PartEdit {
                public:
                    virtual ~PartEdit() = default;

                    /**
                     * @brief Which kind of edit this is
                     */
                    virtual PartEditKind kind() const = 0;

                    /**
                     * @brief Applies this edit to a whole graph. Only called when #kind is ``GraphEdit``
                     *
                     * @param graph The graph to edit
                     * @param ini The associated .ini file, or ``nullptr``
                     * @param modType The type of mod to fix, or ``nullptr``
                     * @param modName The name of the mod to fix to
                     * @param keyFilter
                     @rst
                     Which areas of a part are valid to edit -- ``nullptr`` for "all of them"
                     :raw-html:`<br />` :raw-html:`<br />`

                     A **pointer into the caller's own** filter list, rather than a copy: an
                     implementation forwarding this on to a `Python`_ edit has to hand over the
                     caller's original callable, and the only stable way to recover it from a
                     ``std::function`` is that function's own address (``pybind11``'s
                     ``std::function`` caster re-wraps a callable rather than returning the
                     original -- see `PyGraphGroupEdit`)
                     @endrst
                     *
                     * @param trackKeys
                     @rst
                     The enclosing :cpp:class:`GraphGroupEdit`'s own key-tracking flag, handed down
                     so a graph edit can honour it :raw-html:`<br />` :raw-html:`<br />`

                     A graph edit does its own graph walking (this class hands it the whole graph
                     rather than iterating parts for it, the way it does for a register edit), so it
                     has to be told what the group wanted rather than being able to read a colouring
                     this class built. An edit with its own setting decides how to combine the two
                     @endrst
                     * @param keysToTrack The enclosing :cpp:class:`GraphGroupEdit`'s own key set -- ``std::nullopt`` meaning "every key"
                     *
                     * @return
                     @rst
                     The resulting graph. Normally 'graph' itself; an edit that genuinely produces a
                     *different* graph must return one the enclosing ``graphGroups`` owns
                     @endrst
                     */
                    virtual Graph* editGraph(Graph& graph, IniFile* ini, const ModType* modType, const std::string& modName,
                                              const PartFilter* keyFilter, bool trackKeys,
                                              const std::optional<KeySet>& keysToTrack) = 0;

                    /**
                     * @brief Applies this edit to one part. Only called when #kind is ``RegEdit``
                     *
                     * @param part The part to edit, modified in place
                     * @param sectionName The name of the `section`_ the part belongs to
                     * @param ini The associated .ini file, or ``nullptr``
                     * @param modType The type of mod to fix, or ``nullptr``
                     * @param modName The name of the mod to fix to
                     * @param partRanges The valid order indices to process for 'part'
                     */
                    virtual void editPart(ContentPart& part, const std::string& sectionName, IniFile* ini, const ModType* modType,
                                           const std::string& modName, const OrderRanges& partRanges) = 0;
            };

            /**
             * @brief The edits, key filters and key-tracking settings for one ``.ini`` file's graphs
             */
            struct IniEdits {
                /**
                 * @brief
                 @rst
                 The edits for each graph, keyed by ``(component name, mod object name)``. Applied
                 in the order given
                 @endrst
                 */
                tsl::ordered_map<ModObj, std::vector<PartEdit*>, typename IniGraphGroup<K, V, KeyHash, KeyEqual>::ModObjHash> edits;

                /**
                 * @brief
                 @rst
                 The per-edit part filters for each graph. May be shorter than that graph's edit
                 list -- any edit past the end is unfiltered
                 @endrst
                 */
                tsl::ordered_map<ModObj, std::vector<PartFilter>, typename IniGraphGroup<K, V, KeyHash, KeyEqual>::ModObjHash> keyFilters;

                /**
                 * @brief
                 @rst
                 The `KVP`_ keys to track for each graph. ``std::nullopt`` tracks every key
                 encountered, matching the pure-Python original's ``None``
                 @endrst
                 */
                tsl::ordered_map<ModObj, std::optional<KeySet>, typename IniGraphGroup<K, V, KeyHash, KeyEqual>::ModObjHash> keysToTrack;

                /**
                 * @brief
                 @rst
                 Whether to track `KVPs`_ for each graph. Only consulted when \ref
                 GraphGroupEdit::trackKeysIsGlobal is ``false``
                 @endrst
                 */
                tsl::ordered_map<ModObj, bool, typename IniGraphGroup<K, V, KeyHash, KeyEqual>::ModObjHash> trackKeys;
            };

            /**
             * @brief
             @rst
             The edits to make, one entry per ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

             The pure-Python original splits this across four separate parallel lists
             (``edits``/``keyFilters``/``keysToTrack``/``trackKeys``), each allowed to be a
             different length, with anything past the end falling back to a default. Folding them
             into one entry per ``.ini`` file is equivalent -- only ``edits``'s own length ever
             bounded the iteration
             @endrst
             */
            std::vector<IniEdits> edits;

            /**
             * @brief
             @rst
             Whether \ref trackKeysGlobal applies to every graph, instead of the per-graph
             ``trackKeys`` entries in \ref edits. Matches the pure-Python original's
             ``isinstance(self.trackKeys, bool)`` check
             @endrst
             */
            bool trackKeysIsGlobal = true;

            /**
             * @brief The key-tracking flag used for every graph when \ref trackKeysIsGlobal is ``true``
             */
            bool trackKeysGlobal = false;

            /**
             * @brief Constructs a new graph-group edit
             *
             * @param edits The edits to make, one entry per .ini file
             * @param trackKeysIsGlobal Whether 'trackKeysGlobal' applies to every graph. **Default**: ``true``
             * @param trackKeysGlobal The global key-tracking flag. **Default**: ``false``
             */
            explicit GraphGroupEdit(std::vector<IniEdits> edits = {}, bool trackKeysIsGlobal = true, bool trackKeysGlobal = false);

            /**
             * @brief
             @rst
             Applies 'filters' to one graph, grouping consecutive same-kind edits into a single pass
             @endrst
             *
             * @param graph The graph to edit
             * @param filters The edits to apply, in order
             * @param ini The associated .ini file, or ``nullptr``
             * @param modType The type of mod to fix, or ``nullptr``
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param keyFilters The per-edit part filters. May be shorter than 'filters'. **Default**: empty
             * @param trackKeys Whether to track `KVPs`_ while iterating. **Default**: ``false``
             * @param keysToTrack Which keys to track, or ``std::nullopt`` for all of them. **Default**: ``std::nullopt``
             *
             * @return The edited graph -- 'graph' itself, unless some edit returned a different one
             */
            static Graph* editSectionGraph(Graph& graph, const std::vector<PartEdit*>& filters, IniFile* ini, const ModType* modType,
                                            const std::string& modName = "", const std::vector<PartFilter>& keyFilters = {},
                                            bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt);

            /**
             * @brief Edits every graph named in \ref edits
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param ini The associated original .ini file. **Nullable**
             * @param modType The type of mod to fix. **Nullable**
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& editFromIni(GraphGroups& graphGroups, IniFile* ini, const ModType* modType,
                                      const std::string& modName = "") override;

            /**
             * @brief
             @rst
             Edits every graph named in \ref edits, with no ``.ini`` file to draw state from
             @endrst
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix. **Nullable**
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

        private:
            static Graph* filterGroupEdit(Graph& graph, PartEditKind filterKind, const std::vector<PartEdit*>& filterGroup,
                                           IniFile* ini, const ModType* modType, const std::string& modName,
                                           const std::vector<PartFilter>& keyFilters, bool trackKeys,
                                           const std::optional<KeySet>& keysToTrack);

            GraphGroups& editImpl(GraphGroups& graphGroups, IniFile* ini, const ModType* modType, const std::string& modName);
    };
}

#include "GraphGroupEdit.tpp"

#endif
