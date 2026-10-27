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

#ifndef AGRemapCore_GraphCopy_TPP
#define AGRemapCore_GraphCopy_TPP

#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphCopy<K, V, KeyHash, KeyEqual>::GraphCopy(GraphId srcId, GraphId dstId, Predicate predicate, bool minimal, bool newPartIds):
        srcId(std::move(srcId)), dstId(std::move(dstId)), predicate(std::move(predicate)), minimal(minimal), newPartIds(newPartIds) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphCopy<K, V, KeyHash, KeyEqual>::GraphGroups& GraphCopy<K, V, KeyHash, KeyEqual>::edit(
            GraphGroups& graphGroups, const ModType* modType, const std::string& modName) {
        (void)modType;
        (void)modName;

        const std::size_t groups = graphGroups.size();
        if (srcId.iniIndex >= groups || dstId.iniIndex > groups) {
            return graphGroups;
        }
        Graph* src = graphGroups.getGraph(srcId.iniIndex, srcId.modObj);
        if (src == nullptr) {
            return graphGroups;
        }

        // owned by the groups from here (deepcopyGraph), as GraphCreate's copy is
        Graph* copy = graphGroups.deepcopyGraph(*src, minimal, newPartIds);
        if (copy == nullptr) {
            return graphGroups;
        }
        GraphFilter<K, V, KeyHash, KeyEqual>::filterGraph(*copy, predicate);

        if (dstId.iniIndex == groups) {
            graphGroups.insertGroup(groups);
        }
        graphGroups.addGraph(dstId.iniIndex, dstId.modObj, copy);
        return graphGroups;
    }
}

#endif
