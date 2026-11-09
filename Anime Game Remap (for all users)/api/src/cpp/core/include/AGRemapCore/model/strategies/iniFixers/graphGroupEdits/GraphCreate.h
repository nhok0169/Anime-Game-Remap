#ifndef AGRemapCore_GraphCreate_H
#define AGRemapCore_GraphCreate_H

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

#include <cstddef>
#include <functional>
#include <optional>
#include <string>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Adds a graph to a group of graphs, under a graph id of the caller's choosing -- the counterpart
     of :cpp:class:`GraphRemove`, which takes one out :raw-html:`<br />` :raw-html:`<br />`

     Where :cpp:class:`GraphGroupRemap` produces its graphs by copying ones the group already holds,
     this one takes a graph the CALLER built and puts it in. That is what a fixer needs when the
     graph it wants to render is not a copy of anything the parser handed it

     .. note::
        **The graph is BORROWED and deep-copied in.** :cpp:func:`IIniGraphGroups::addGraph` requires
        a graph the groups object already owns -- one that came out of its own
        :cpp:func:`IIniGraphGroups::getGraph`, :cpp:func:`IIniGraphGroups::removeGraph`,
        :cpp:func:`IIniGraphGroups::deepcopyGraph` or :cpp:func:`IIniGraphGroups::createGraph` --
        because the groups own their graphs' lifetimes. So this edit hands \ref graph to
        :cpp:func:`IIniGraphGroups::deepcopyGraph` and adds the COPY, which leaves the caller owning
        what it passed and free to reuse it for several ids. Storing the caller's pointer directly
        would be a dangling reference the moment the caller's graph went out of scope

     .. note::
        A \ref graphId whose ``iniIndex`` is past the end of the groups is skipped silently, as
        :cpp:class:`GraphRemove` skips an out-of-range id -- **except** for an index of exactly
        :cpp:func:`IIniGraphGroups::size`, which appends a fresh group and adds the graph to that.
        Appending the ONE group the index asks for is the useful case (a fix that renders an extra
        ``.ini`` file); growing the list to reach an arbitrary index is not, and an index far past the
        end is a caller's mistake rather than a request for thousands of empty groups

     .. note::
        An existing graph already stored under \ref graphId is REPLACED, which is
        :cpp:func:`IIniGraphGroups::addGraph`'s own behaviour rather than anything this edit adds
     @endrst
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphCreate: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
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
             * @copydoc BaseIniGraphGroupEdit::GraphId
             */
            using GraphId = typename Base::GraphId;

            /**
             * @brief Where the graph goes -- the ``.ini`` index, the component name and the mod object name
             */
            GraphId graphId;

            /**
             * @brief
             @rst
             The graph to add, **borrowed** -- deep-copied into the group rather than stored, see this
             class's own note. ``nullptr`` makes the edit a no-op
             @endrst
             */
            Graph* graph;

            /**
             * @brief
             @rst
             The graph to add, asked for when the edit runs -- set, it is used instead of \ref graph, for a graph
             that only exists by then (eg. a parser's :cpp:func:`GIMIParser::globalGraph`, built during the
             parse). What it returns is borrowed and deep-copied as \ref graph is; ``nullptr`` makes the edit a
             no-op. **Default**: unset
             @endrst
             */
            std::function<Graph*()> graphOf;

            /**
             * @brief Whether the copy taken of \ref graph is a minimal one
             */
            bool minimal;

            /**
             * @brief Whether the parts of the copy taken of \ref graph get fresh ids
             */
            bool newPartIds;

            /**
             * @brief A `section`_ a graph is built from -- see \ref sectionsOf
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The `sections`_ of each group's graph, given the group's index -- set, the edit builds a graph from
             them for EVERY group present when it runs and adds it under \ref graphId's component and object
             (its ``iniIndex`` is unused), instead of adding \ref graph. **Borrowed**, read only while the edit runs.
             A section of a name already in the graph is left out, so the first one wins; a group handed no
             sections gets no graph. When there are no groups at all, group 0's sections go into a group of their
             own. This is how sections held in a :cpp:class:`GraphEditVariables` variable are written out as a
             graph in each group -- a fixer's own sections into its file and every copy. **Default**: unset
             @endrst
             */
            std::function<std::vector<Section*>(std::size_t group)> sectionsOf;

            /**
             * @brief How a graph built from \ref sectionsOf follows ``run =``. **Default**: :cpp:func:`runCalls`
             */
            IfTemplateRunConfig<K, V> runConfig = runCalls();

            /**
             * @brief The context a graph built from \ref sectionsOf builds its conditions in, or ``nullptr`` for its own. **Default**: ``nullptr``
             */
            Z3Context* z3 = nullptr;

            /**
             * @brief
             @rst
             Objects a graph of any of which, in ANY group, stops the edit: nothing is added when one is there -- eg. a section
             that hides a target slot, added only when nothing is remapped onto that slot. Empty (the default) always adds
             @endrst
             */
            std::vector<ModObj> onlyWithout;

            /**
             * @brief
             @rst
             Objects a graph of any of which, in ANY group, is what lets the edit add: nothing is added when no group holds
             one -- eg. a texture list a fix writes for a component, added only when the mod draws that component, or the
             fix's own sections, added when the mod draws any of the source's components. The mirror of \ref onlyWithout.
             Empty (the default) always adds
             @endrst
             */
            std::vector<ModObj> onlyWith;

            /**
             * @brief How a graph of `sections`_ a fix writes follows ``run =``: each value names the `section`_ it calls
             */
            static IfTemplateRunConfig<K, V> runCalls();

            /**
             * @brief Constructs a new graph-adding edit
             *
             * @param graphId Where the graph goes
             * @param graph The graph to add, borrowed and deep-copied in. **Nullable** -- a no-op
             * @param minimal Whether the copy is a minimal one. **Default**: ``true``
             * @param newPartIds Whether the copy's parts get fresh ids. **Default**: ``true``
             */
            GraphCreate(GraphId graphId, Graph* graph, bool minimal = true, bool newPartIds = true);

            /**
             * @brief Adds a copy of \ref graph to 'graphGroups' under \ref graphId
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

#include "GraphCreate.tpp"

#endif
