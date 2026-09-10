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
