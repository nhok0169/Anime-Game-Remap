#ifndef AGRemapCore_IIniGraphGroups_H
#define AGRemapCore_IIniGraphGroups_H

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The sequence of :cpp:class:`IniGraphGroup`\\s (one per ``.ini`` file) that every
     ``graphGroupEdits/`` edit operates on, behind an interface :raw-html:`<br />`
     :raw-html:`<br />`

     **Why this interface exists at all.** Every other ported subsystem in this codebase could
     name its container concretely, because both callers agreed on it. This one can't: a plain C++
     caller's groups are a ``std::vector<IniGraphGroup<K, V>>`` (real C++ values, owned by the
     vector), while the `Python`_-facing groups are a `Python`_ ``list`` of ``IniGraphGroup``
     objects whose ``graphs`` really *is* a `Python`_ ``dict`` (``PyIniGraphGroup`` -- deliberately
     so, because ``GIMIParser.py`` depends on that dict's *reference* semantics; see
     :cpp:class:`IniGraphGroup`'s own note). Converting one into the other at the binding boundary
     would silently break that aliasing, so instead the algorithms are written against this
     interface and each side supplies its own implementation:
     :cpp:class:`IniGraphGroupsVec` here in the core, and ``PyIniGraphGroups`` in the binding
     layer.

     :raw-html:`<br />`

     **Ownership contract.** *The implementation owns every graph it hands out*, and every
     ``Graph*`` returned by any method below stays valid for as long as this object lives. Core
     algorithms therefore only ever hold borrowed ``Graph*``\\s -- they never construct, copy or
     destroy an :cpp:class:`IniSectionGraph` themselves. That's not a stylistic choice: the
     `Python`_ implementation's graphs are `Python`_ objects with their own keep-alive bookkeeping
     (``PyIniSectionGraph::refreshKeepAlive``/``z3CtxKeepAlive``), which cannot be reconstructed
     from a bare C++ graph after the fact -- so graph *creation* has to be a method on this
     interface (#deepcopyGraph / #createGraph), not something a core algorithm does for itself.

     :raw-html:`<br />`

     .. note::
        #removeGraph deliberately returns a still-valid borrowed pointer rather than transferring
        ownership: ``GraphGroupRemap`` removes a graph and then keeps using it (as the source of a
        deep copy) after it is no longer in any group. Implementations keep removed graphs alive
        until the whole object is destroyed
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IIniGraphGroups {
        public:

            /**
             * @brief The type of graph held in a group
             */
            using Graph = IniSectionGraph<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The type of `section`_ a #Graph is built out of
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc IniGraphGroup::ModObj
             */
            using ModObj = std::pair<std::string, std::string>;

            virtual ~IIniGraphGroups() = default;

            /**
             * @brief The number of groups (ie. the number of .ini files)
             */
            virtual std::size_t size() const = 0;

            /**
             * @brief
             @rst
             Inserts a fresh, empty group at 'groupInd', shifting every later group up by one
             @endrst
             *
             * @param groupInd Where to insert. Clamped to #size for an out-of-range index
             */
            virtual void insertGroup(std::size_t groupInd) = 0;

            /**
             * @brief Removes the group at 'groupInd', if it exists
             *
             * @param groupInd Which group to remove
             */
            virtual void removeGroup(std::size_t groupInd) = 0;

            /**
             * @brief
             @rst
             The ``(component name, mod object name)`` key of every graph in the group at
             'groupInd', in insertion order -- empty for an out-of-range index
             @endrst
             *
             * @param groupInd Which group to read
             */
            virtual std::vector<ModObj> modObjs(std::size_t groupInd) const = 0;

            /**
             * @brief The number of graphs in the group at 'groupInd' (``0`` for an out-of-range index)
             *
             * @param groupInd Which group to read
             */
            virtual std::size_t graphCount(std::size_t groupInd) const = 0;

            /**
             * @brief The graph stored under 'modObj' in the group at 'groupInd', or ``nullptr``
             *
             * @param groupInd Which group to read
             * @param modObj The associated component and mod object for the graph
             */
            virtual Graph* getGraph(std::size_t groupInd, const ModObj& modObj) const = 0;

            /**
             * @brief
             @rst
             Adds 'graph' under 'modObj' in the group at 'groupInd', replacing whatever was already
             stored there -- a no-op for an out-of-range index
             @endrst
             *
             * @param groupInd Which group to add to
             * @param modObj The associated component and mod object for the graph
             * @param graph
             @rst
             The graph to add. Must be one this object already owns (ie. came out of #getGraph,
             #removeGraph, #deepcopyGraph or #createGraph) -- see this class's ownership contract
             @endrst
             */
            virtual void addGraph(std::size_t groupInd, const ModObj& modObj, Graph* graph) = 0;

            /**
             * @brief Removes the graph stored under 'modObj' in the group at 'groupInd'
             *
             * @param groupInd Which group to remove from
             * @param modObj The associated component and mod object for the graph
             *
             * @return
             @rst
             The removed graph -- still valid and still owned by this object (see this class's own
             note) -- or ``nullptr`` if there was nothing to remove
             @endrst
             */
            virtual Graph* removeGraph(std::size_t groupInd, const ModObj& modObj) = 0;

            /**
             * @brief Deep-copies an existing graph, taking ownership of the copy
             *
             * @param src The graph to copy
             * @param minimal Whether to make a minimal copy. **Default**: ``true``
             * @param newPartIds Whether the copied parts get fresh ids. **Default**: ``true``
             *
             * @return The new copy. Owned by this object, and not in any group until #addGraph is called for it
             */
            virtual Graph* deepcopyGraph(const Graph& src, bool minimal = true, bool newPartIds = true) = 0;

            /**
             * @brief Builds a brand-new graph, taking ownership of it
             *
             * @param sections All the `sections`_ available to the graph, borrowed unless 'copySections' is ``true``
             * @param targetSectionNames The names of the `sections`_ the new subgraph should reach
             * @param copySections Whether to deep-copy the referenced `sections`_. **Default**: ``false``
             * @param z3Ctx The `Z3`_ context the new graph's predicates belong to. **Default**: ``nullptr``
             *
             * @return The new graph. Owned by this object, and not in any group until #addGraph is called for it
             */
            virtual Graph* createGraph(std::unordered_map<std::string, Section*> sections, std::vector<std::string> targetSectionNames,
                                        bool copySections = false, Z3Context* z3Ctx = nullptr) = 0;
    };


    /**
     * @brief
     @rst
     The plain-C++ :cpp:class:`IIniGraphGroups`, backed by a ``std::vector`` of real
     :cpp:class:`IniGraphGroup` values :raw-html:`<br />` :raw-html:`<br />`

     Use this from any standalone C++ caller. The `pybind11`_ layer uses its own
     ``PyIniGraphGroups`` implementation instead -- see :cpp:class:`IIniGraphGroups`'s own note for
     why there are two
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IniGraphGroupsVec: public IIniGraphGroups<K, V, KeyHash, KeyEqual> {
        public:
            using Base = IIniGraphGroups<K, V, KeyHash, KeyEqual>;
            using Graph = typename Base::Graph;
            using Section = typename Base::Section;
            using ModObj = typename Base::ModObj;

            /**
             * @brief The type of group this holds
             */
            using Group = IniGraphGroup<K, V, KeyHash, KeyEqual>;

            /**
             * @brief Constructs a view over 'groups'
             *
             * @param groups The groups to operate on, **borrowed** -- this object edits them in place and does not own them
             * @param runConfig The domain customization points any graph built by #createGraph uses (see :cpp:class:`IfTemplateRunConfig`)
             */
            explicit IniGraphGroupsVec(std::vector<Group>& groups, IfTemplateRunConfig<K, V> runConfig);

            std::size_t size() const override;
            void insertGroup(std::size_t groupInd) override;
            void removeGroup(std::size_t groupInd) override;
            std::vector<ModObj> modObjs(std::size_t groupInd) const override;
            std::size_t graphCount(std::size_t groupInd) const override;
            Graph* getGraph(std::size_t groupInd, const ModObj& modObj) const override;
            void addGraph(std::size_t groupInd, const ModObj& modObj, Graph* graph) override;
            Graph* removeGraph(std::size_t groupInd, const ModObj& modObj) override;
            Graph* deepcopyGraph(const Graph& src, bool minimal = true, bool newPartIds = true) override;
            Graph* createGraph(std::unordered_map<std::string, Section*> sections, std::vector<std::string> targetSectionNames,
                                bool copySections = false, Z3Context* z3Ctx = nullptr) override;

        private:
            std::vector<Group>& groups_;
            IfTemplateRunConfig<K, V> runConfig_;

            // Graphs this view produced (#deepcopyGraph/#createGraph) or removed from a group,
            // kept alive here for as long as this view lives -- see IIniGraphGroups's ownership
            // contract and its note on removeGraph.
            std::vector<std::unique_ptr<Graph>> owned_;
    };
}

#include "IIniGraphGroups.tpp"

#endif
