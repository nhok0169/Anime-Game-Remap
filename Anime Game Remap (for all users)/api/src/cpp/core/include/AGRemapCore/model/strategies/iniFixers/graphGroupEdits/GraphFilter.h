// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#ifndef AGRemapCore_GraphFilter_H
#define AGRemapCore_GraphFilter_H

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Keeps only part of some graphs, in place: every `KVP`_ that \ref predicate rejects is removed
     from the graph's `sections`_ :raw-html:`<br />` :raw-html:`<br />`

     The `section`_'s ``if`` / ``else`` structure is kept as it is, so a `KVP`_ that survives
     stays under the same condition it was written under -- a draw under a mod's toggle stays under
     that toggle. :cpp:class:`GraphCopy` makes the same selection on a COPY of a graph instead

     .. code-block:: ini

        ; kept: the KVPs a predicate accepting ib / vb0 / drawindexed keeps
        [TextureOverrideLegsA]
        hash = d8003df3          ; removed
        if $swapvar == 1
            ib = ResourceBodyIB
            vb0 = ResourceBodyVB
            drawindexed = auto
        endif

     .. note::
        A `section`_ a removed ``run =`` used to call stays in the graph: the selection is made per
        `KVP`_, and what the graph reaches is not recomputed
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphFilter: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
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
             * @copydoc BaseIniGraphGroupEdit::GraphId
             */
            using GraphId = typename Base::GraphId;

            /**
             * @copydoc BaseIniGraphGroupEdit::Graph
             */
            using Graph = typename Base::Graph;

            /**
             * @brief A part holding `KVPs`_
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @brief A `section`_ of the graph
             */
            using Section = typename Graph::Section;

            /**
             * @brief
             @rst
             Decides whether one `KVP`_ is kept. Takes, in order:

             #. The `section`_ the `KVP`_ is in -- the whole `section`_, so the decision can depend on
                what runs before it, in any part (eg. what ``vb0`` was bound to when a
                ``drawindexed`` runs)
             #. The part holding it
             #. The `KVP`_'s index within that part

             Every decision is made before anything is removed, so a predicate always sees the
             part as it was
             @endrst
             */
            using Predicate = std::function<bool(const Section&, const ContentPart&, std::size_t)>;

            /**
             * @brief The ids of the graphs to filter
             */
            std::vector<GraphId> graphIds;

            /**
             * @brief Decides which `KVPs`_ are kept. An empty predicate keeps everything
             */
            Predicate predicate;

            /**
             * @brief Constructs a new filtering edit
             *
             * @param graphIds The ids of the graphs to filter
             * @param predicate Decides which `KVPs`_ are kept
             */
            explicit GraphFilter(std::vector<GraphId> graphIds = {}, Predicate predicate = nullptr);

            /**
             * @brief Removes, from every graph named by \ref graphIds, the `KVPs`_ \ref predicate rejects
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix. Unused by this edit
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

            /**
             * @brief Removes from 'graph' every `KVP`_ 'predicate' rejects
             *
             * @param graph The graph to filter, in place
             * @param predicate Decides which `KVPs`_ are kept. Empty keeps everything
             *
             * @return How many `KVPs`_ were removed
             */
            static std::size_t filterGraph(Graph& graph, const Predicate& predicate);
    };
}

#include "GraphFilter.tpp"

#endif
