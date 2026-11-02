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

#ifndef AGRemapCore_GraphCreate_TPP
#define AGRemapCore_GraphCreate_TPP

#include <utility>


#include <unordered_map>
#include <vector>

namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphCreate<K, V, KeyHash, KeyEqual>::GraphCreate(GraphId graphId, Graph* graph, bool minimal, bool newPartIds):
        graphId(std::move(graphId)), graph(graph), minimal(minimal), newPartIds(newPartIds) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    IfTemplateRunConfig<K, V> GraphCreate<K, V, KeyHash, KeyEqual>::runCalls() {
        return IfTemplateRunConfig<K, V>{IniKeywords::Run, [](const V& val) { return val; },
                                         [](const std::string& section) { return section; }};
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphCreate<K, V, KeyHash, KeyEqual>::GraphGroups& GraphCreate<K, V, KeyHash, KeyEqual>::edit(
            GraphGroups& graphGroups, const ModType* modType, const std::string& modName) {
        (void)modType;
        (void)modName;

        if (sectionsOf) {
            // a graph per group, of the sections it is handed; and group 0's into a group of its own when there is
            // none (every group removed, say)
            const std::size_t count = graphGroups.size() > 0 ? graphGroups.size() : std::size_t(1);
            for (std::size_t g = 0; g < count; ++g) {
                std::unordered_map<std::string, Section*> byName;
                std::vector<std::string> names;
                for (Section* section : sectionsOf(g)) {
                    if (section != nullptr && byName.emplace(section->name, section).second) {
                        names.push_back(section->name);
                    }
                }
                if (names.empty()) {
                    continue;
                }
                if (g == graphGroups.size()) {
                    graphGroups.insertGroup(g);
                }

                Graph built(std::move(byName), names, runConfig, true, false, z3);
                if (Graph* owned = graphGroups.deepcopyGraph(built, minimal, newPartIds)) {
                    graphGroups.addGraph(g, graphId.modObj, owned);
                }
            }
            return graphGroups;
        }

        Graph* source = graphOf ? graphOf() : graph;
        if (source == nullptr) {
            return graphGroups;
        }

        const std::size_t groups = graphGroups.size();
        if (graphId.iniIndex > groups) {
            // Past the end by more than one: skipped silently, as GraphRemove skips an out-of-range
            // id. This also covers the "no such graph" sentinel a negative index parses to on the
            // Python side, which must not be read as a request for that many empty groups.
            //
            // An EARLY-OUT rather than the thing that makes the skip happen: addGraph is itself a
            // no-op for an out-of-range index, so removing this return changes no observable
            // behaviour -- measured, by mutating it and watching every test still pass. What it
            // saves is a deepcopyGraph whose result nothing would ever bind, and which the groups
            // would then own for the rest of the view's life.
            return graphGroups;
        }

        if (graphId.iniIndex == groups) {
            graphGroups.insertGroup(groups);
        }

        // Deep-copied rather than added directly: addGraph requires a graph the groups already own,
        // because they own their graphs' lifetimes. See this class's own note -- adding the caller's
        // pointer would dangle the moment the caller's graph went out of scope, and the copy leaves
        // the caller free to reuse one graph for several ids.
        Graph* owned = graphGroups.deepcopyGraph(*source, minimal, newPartIds);
        if (owned == nullptr) {
            return graphGroups;
        }

        graphGroups.addGraph(graphId.iniIndex, graphId.modObj, owned);
        return graphGroups;
    }
}

#endif
