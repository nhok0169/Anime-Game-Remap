#ifndef AGRemapCore_GraphGroupRemap_H
#define AGRemapCore_GraphGroupRemap_H

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Remaps the graphs from a group of graphs :raw-html:`<br />` :raw-html:`<br />`

     Each source graph is removed from wherever it currently lives, and one fresh graph is created
     per remap target (see \ref remapGraphs). A target that collides with a graph already present in
     the destination ``.ini`` file's group lands in an **additional** group for that same ``.ini``
     file instead of overwriting it -- which is why this is the one edit in this family that changes
     how many :cpp:class:`IniGraphGroup`\\s there are
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GraphGroupRemap: public BaseIniGraphGroupEdit<K, V, KeyHash, KeyEqual> {
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
             * @brief
             @rst
             Renames one `section`_ -- takes the old name, returns the new one. An empty
             ``std::function`` stands in for the pure-Python original's "no rename function given",
             which falls back to :cpp:func:`IniNamingTools::getObjRemapFixName`
             @endrst
             */
            using RenameFunc = std::function<std::string(const std::string&)>;

            /**
             * @brief One destination for a remapped graph
             */
            struct RemapTarget {
                /**
                 * @brief
                 @rst
                 Where the remapped graph goes :raw-html:`<br />` :raw-html:`<br />`

                 .. note::
                    ``iniIndex`` may not end up being the index of the group that actually holds the
                    graph -- see \ref remapGraphs's own note on collisions
                 @endrst
                 */
                GraphId id;

                /**
                 * @brief The optional rename function for this target -- empty for "use the default"
                 */
                RenameFunc renameFunc;

                RemapTarget() = default;

                /**
                 * @brief Constructs a remap target
                 *
                 * @param id Where the remapped graph goes
                 * @param renameFunc The optional rename function. **Default**: empty
                 */
                explicit RemapTarget(GraphId id, RenameFunc renameFunc = {}): id(std::move(id)), renameFunc(std::move(renameFunc)) {}
            };

            /**
             * @brief
             @rst
             The remap for the graphs -- an ordered sequence of ``(source graph, its targets)``
             pairs :raw-html:`<br />` :raw-html:`<br />`

             A ``std::vector`` of pairs rather than a map, deliberately: the pure-Python original
             is a `Python`_ ``dict`` whose *iteration order* decides the order remapped graphs are
             created in (and therefore which target wins a collision), so the ordering has to be
             part of the type rather than an incidental property of a hash container
             @endrst
             */
            using RemapList = std::vector<std::pair<GraphId, std::vector<RemapTarget>>>;

            /**
             * @brief
             @rst
             Builds the new graph for one remap target :raw-html:`<br />` :raw-html:`<br />`

             The returned graph must be one 'graphGroups' already owns (see
             :cpp:class:`IIniGraphGroups`'s ownership contract) -- in practice, whatever
             :cpp:func:`IIniGraphGroups::deepcopyGraph` handed back. Returning ``nullptr`` skips
             that target
             @endrst
             */
            using CreateToGraph = std::function<Graph*(GraphGroups& graphGroups, Graph& fromGraph, const GraphId& fromId,
                                                        const GraphId& toId, const RenameFunc& renameFunc)>;

            /**
             * @brief The remap for the graphs
             */
            RemapList remap;

            /**
             * @brief Constructs a new graph-remapping edit
             *
             * @param remap The remap for the graphs
             */
            explicit GraphGroupRemap(RemapList remap = {});

            /**
             * @brief
             @rst
             Remaps the graphs from a group of graphs :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A target whose ``(component, object)`` key is already taken in the destination
                ``.ini`` file's group goes into an **additional** group for that same ``.ini`` file
                (created on demand), rather than overwriting the existing graph. Groups end up
                ordered by ``.ini`` file, then by the order they were created

             .. note::
                A target ``iniIndex`` past the end of 'graphGroups' is clamped to one-past-the-last
                original ``.ini`` file -- a dedicated trailing bucket that exists precisely for
                "remap this into a brand-new ``.ini`` file"

             .. note::
                A source ``.ini`` file whose original group is left with no graphs at all is dropped
                entirely
             @endrst
             *
             * @param graphGroups The group of graphs to remap, modified in place
             * @param createToGraph Builds the new graph for one remap target
             *
             * @return The same groups that were passed in, after remapping
             */
            GraphGroups& remapGraphs(GraphGroups& graphGroups, const CreateToGraph& createToGraph);

            /**
             * @brief
             @rst
             The default #CreateToGraph -- deep-copies 'fromGraph' and renames every `section`_ in
             the copy
             @endrst
             *
             * @param graphGroups The group of graphs the copy is registered with
             * @param fromGraph The graph to copy
             * @param modObj The id of the graph being copied from
             * @param newModObj The id of the graph being copied to
             * @param renameFunc
             @rst
             The rename function to use. When empty, falls back to
             :cpp:func:`IniNamingTools::getObjRemapFixName` against 'modObj'/'newModObj'
             @endrst
             * @param modName The name of the mod to fix to, used by that fallback. **Default**: ``""``
             *
             * @return The new, renamed copy
             */
            static Graph* copyGraph(GraphGroups& graphGroups, Graph& fromGraph, const GraphId& modObj, const GraphId& newModObj,
                                     const RenameFunc& renameFunc = {}, const std::string& modName = "");

            /**
             * @brief Remaps the graphs, building each new graph with \ref copyGraph
             *
             * @param graphGroups The group of graphs to remap, modified in place
             * @param modType The type of mod to fix. Unused by this edit
             * @param modName The name of the mod to fix to, handed to \ref copyGraph. **Default**: ``""``
             *
             * @return The same groups that were passed in, after remapping
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;
    };
}

#include "GraphGroupRemap.tpp"

#endif
