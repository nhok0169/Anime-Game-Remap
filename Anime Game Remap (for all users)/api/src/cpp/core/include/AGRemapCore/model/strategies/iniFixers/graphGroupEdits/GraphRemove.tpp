#ifndef AGRemapCore_GraphRemove_TPP
#define AGRemapCore_GraphRemove_TPP

#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphRemove<K, V, KeyHash, KeyEqual>::GraphRemove(std::vector<GraphId> graphIds): graphIds(std::move(graphIds)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphRemove<K, V, KeyHash, KeyEqual>::GraphGroups& GraphRemove<K, V, KeyHash, KeyEqual>::edit(GraphGroups& graphGroups, const ModType* modType,
                                                                                                           const std::string& modName) {
        (void)modType;
        (void)modName;

        std::size_t graphGroupsLen = graphGroups.size();

        for (const GraphId& graphId : graphIds) {
            // An out-of-range .ini index is skipped rather than clamped -- matching the
            // pure-Python original's own "if (iniInd >= graphGroupsLen): continue".
            if (graphId.iniIndex >= graphGroupsLen) {
                continue;
            }

            graphGroups.removeGraph(graphId.iniIndex, graphId.modObj);
        }

        return graphGroups;
    }
}

#endif
