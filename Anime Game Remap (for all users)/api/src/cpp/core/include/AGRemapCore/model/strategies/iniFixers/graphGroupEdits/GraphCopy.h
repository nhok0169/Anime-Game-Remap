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

#ifndef AGRemapCore_GraphCopy_H
#define AGRemapCore_GraphCopy_H

#include <functional>
#include <string>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphFilter.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Copies one graph of a group of graphs to another graph id -- optionally only the part of it
     \ref predicate keeps (the same selection :cpp:class:`GraphFilter` makes in place)
     :raw-html:`<br />` :raw-html:`<br />`

     The copy is a deep copy, so editing it never touches the source. Its `sections`_ keep the
     source's names: rename them (:cpp:class:`GraphRename`) before both graphs are rendered into
     one file. To compose the copy into another graph, follow with :cpp:class:`GraphInherit`.

     .. note::
        The destination follows :cpp:class:`GraphCreate`'s rule: an ``iniIndex`` equal to the number
        of groups appends one fresh group, one further past the end is skipped, and an existing
        graph at the destination is replaced. A source id naming no graph copies nothing
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphCopy: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
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
             * @copydoc GraphFilter::Predicate
             */
            using Predicate = typename GraphFilter<K, V, KeyHash, KeyEqual>::Predicate;

            /**
             * @brief The id of the graph to copy
             */
            GraphId srcId;

            /**
             * @brief The id the copy is stored under
             */
            GraphId dstId;

            /**
             * @brief Decides which `KVPs`_ of the copy are kept. Empty copies the whole graph
             */
            Predicate predicate;

            /**
             * @brief Whether the copy's part ids are renumbered minimally -- see :cpp:func:`IniSectionGraph::deepcopy`
             */
            bool minimal;

            /**
             * @brief Whether the copy's parts get new ids -- see :cpp:func:`IniSectionGraph::deepcopy`
             */
            bool newPartIds;

            /**
             * @brief Constructs a new copying edit
             *
             * @param srcId The id of the graph to copy
             * @param dstId The id the copy is stored under
             * @param predicate Decides which `KVPs`_ of the copy are kept. **Default**: everything
             * @param minimal See \ref minimal. **Default**: ``true``
             * @param newPartIds See \ref newPartIds. **Default**: ``true``
             */
            GraphCopy(GraphId srcId, GraphId dstId, Predicate predicate = nullptr, bool minimal = true, bool newPartIds = true);

            /**
             * @brief Stores a (filtered) copy of the graph at \ref srcId under \ref dstId
             *
             * @param graphGroups The group of graphs to edit for each .ini file, modified in place
             * @param modType The type of mod to fix. Unused by this edit
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             *
             * @return The same groups that were passed in, after editing
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;
    };
}

#include "GraphCopy.tpp"

#endif
