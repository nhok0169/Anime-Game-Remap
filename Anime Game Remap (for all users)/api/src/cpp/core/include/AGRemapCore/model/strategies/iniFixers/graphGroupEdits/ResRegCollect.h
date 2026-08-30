#ifndef AGRemapCore_ResRegCollect_H
#define AGRemapCore_ResRegCollect_H

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Creates the :cpp:class:`IniSectionGraph` for a particular resource :raw-html:`<br />`
     :raw-html:`<br />`

     Runs in three phases. First it **collects** every reference to the resource -- walking each
     source graph named in \ref srcRegs and recording where the register naming the resource
     appears. Then, optionally, it **remaps** those source graphs (see \ref remaps). Finally, for
     each resource subtype in \ref resEdits, it works out the fixed name of every collected
     reference, rewrites it in place, and hands the collected set to that subtype's
     :cpp:class:`BaseResEdit` to build the resource's own graph and models
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class ResRegCollect: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
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
             * @copydoc BaseIniGraphGroupEdit::GraphId
             */
            using GraphId = typename Base::GraphId;

            /**
             * @copydoc BaseIniGraphGroupEdit::GraphIdHash
             */
            using GraphIdHash = typename Base::GraphIdHash;

            /**
             * @brief The per-part iteration data the predicates are handed
             */
            using IterData = SectionIterData<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ranges of valid `KVP`_ order indices a \ref PartPredicate returns
             */
            using OrderRanges = Ranges<long long>;

            /**
             * @brief The resource edit each resource subtype is built with
             */
            using ResEdit = BaseResEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ``.ini`` file the resources are built for
             */
            using Context = typename ResEdit::Context;

            /**
             * @brief The graph remapper this uses when \ref remaps is set
             */
            using Remapper = GraphGroupRemap<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc GraphGroupRemap::RenameFunc
             */
            using RenameFunc = typename Remapper::RenameFunc;

            /**
             * @brief
             @rst
             Restricts collection to specific order indices of a :cpp:class:`IfContentPart` -- an
             empty ``std::function`` means the whole part
             @endrst
             */
            using PartPredicate = std::function<OrderRanges(const IterData&)>;

            /**
             * @brief
             @rst
             Decides whether one reference to the resource should be collected -- takes the register
             name holding the reference, the name of the resource being referenced, and the part it
             was found in. An empty ``std::function`` accepts everything
             @endrst
             */
            using ResPredicate = std::function<bool(const K&, const V&, const IterData&)>;

            /**
             * @brief A map keyed by which graph something applies to
             */
            template <typename T>
            using ByGraph = tsl::ordered_map<GraphId, T, GraphIdHash>;

            /**
             * @brief
             @rst
             The different registers that reference the particular resource, keyed by which
             :cpp:class:`IniSectionGraph` to search
             @endrst
             */
            ByGraph<K> srcRegs;

            /**
             * @brief
             @rst
             Describes how a resource should be built, keyed by the name of the resource's subtype
             :raw-html:`<br />` :raw-html:`<br />`

             The edits are **borrowed**, not owned -- they routinely outlive one edit run and are
             shared with whatever constructed them
             @endrst
             */
            tsl::ordered_map<std::string, ResEdit*> resEdits;

            /**
             * @brief Which order indices to collect from, keyed by which graph the predicate applies to
             */
            ByGraph<PartPredicate> partPredicates;

            /**
             * @brief Which references to collect, keyed by which graph the predicate applies to
             */
            ByGraph<ResPredicate> resPredicates;

            /**
             * @brief
             @rst
             Whether to remap the graphs searched from \ref srcRegs, keyed first by the source graph
             and then by the resource subtype :raw-html:`<br />` :raw-html:`<br />`

             Empty means "no remapping at all", which is a genuinely different case from "remap
             nothing" -- see \ref edit
             @endrst
             */
            ByGraph<tsl::ordered_map<std::string, typename Remapper::RemapTarget>> remaps;

            /**
             * @brief Whether \ref trackKeysGlobal applies to every graph, instead of the per-graph \ref trackKeys entries
             */
            bool trackKeysIsGlobal = true;

            /**
             * @brief The key-tracking flag used for every graph when \ref trackKeysIsGlobal is ``true``
             */
            bool trackKeysGlobal = false;

            /**
             * @brief Whether to track `KVPs`_ while searching each graph, keyed by which graph
             */
            ByGraph<bool> trackKeys;

            /**
             * @brief
             @rst
             Which `KVP`_ keys to track while searching each graph. A missing entry (or
             ``std::nullopt``) tracks every key encountered
             @endrst
             */
            ByGraph<std::optional<std::unordered_set<K, KeyHash, KeyEqual>>> keysToTrack;

            /**
             * @brief One collected reference to the resource
             */
            struct ResCall {
                /**
                 * @brief The order index the reference was found at within its part
                 */
                long long orderInd = 0;

                /**
                 * @brief The name of the resource `section`_ being referenced
                 */
                V val;
            };

            /**
             * @brief
             @rst
             The calls to the resource, keyed by source graph, then `section`_ name, then the id of
             the part within that `section`_ :raw-html:`<br />` :raw-html:`<br />`

             Rebuilt from scratch by every \ref edit, and cleared again afterwards by
             \ref editFromIni -- it is scratch state, not configuration
             @endrst
             */
            ByGraph<tsl::ordered_map<std::string, tsl::ordered_map<std::size_t, std::vector<ResCall>>>> resCalls;

            /**
             * @brief Constructs a new resource-collecting edit
             *
             * @param srcRegs The registers that reference the resource, keyed by which graph to search
             * @param resEdits How each resource subtype is built. Borrowed, not owned
             */
            ResRegCollect(ByGraph<K> srcRegs = {}, tsl::ordered_map<std::string, ResEdit*> resEdits = {});

            /**
             * @brief Clears \ref resCalls and every resource edit's own saved state
             */
            void clear() override;

            /**
             * @brief Collects, remaps and builds -- see this class's own description
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param ctx The .ini file the resources are built for
             * @param modType The type of mod to fix. Unused directly -- reaches the resource edits only
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& editWithContext(GraphGroups& graphGroups, Context& ctx, const ModType* modType,
                                          const std::string& modName = "");

            /**
             * @brief
             @rst
             Collects and remaps, but builds nothing -- there is no ``.ini`` file to build resources
             for :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Faithful to the pure-Python original, whose own ``_buildResource`` is a no-op when
                ``ini`` is ``None``
             @endrst
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix
             * @param modName The name of the mod to fix to. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

        private:
            // What a remapped source graph turned into, keyed by source graph then resource subtype.
            struct RemappedGraph {
                Graph* graph = nullptr;
                RenameFunc renameFunc;
                bool partIdRefreshRequired = false;
            };

            using RemappedGraphs = ByGraph<tsl::ordered_map<std::string, RemappedGraph>>;

            void collectFromGraphGroup(GraphGroups& graphGroups, const GraphId& srcModObj, const K& srcReg);
            GraphGroups& remapGraphs(GraphGroups& graphGroups, RemappedGraphs* remappedGraphs);
            void collectResourceNames(const std::string& resSubType, ResEdit& resEdit,
                                       typename ResEdit::CollectedSections& collectedSections, GraphGroups& graphGroups,
                                       RemappedGraphs* remappedGraphs, bool editGraph, const std::string& modName);
            GraphGroups& editImpl(GraphGroups& graphGroups, Context* ctx, const ModType* modType, const std::string& modName);
    };
}

#include "ResRegCollect.tpp"

#endif
