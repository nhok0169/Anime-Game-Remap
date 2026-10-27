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

#ifndef AGRemapCore_GraphFilter_TPP
#define AGRemapCore_GraphFilter_TPP

#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphFilter<K, V, KeyHash, KeyEqual>::GraphFilter(std::vector<GraphId> graphIds, Predicate predicate):
        graphIds(std::move(graphIds)), predicate(std::move(predicate)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphFilter<K, V, KeyHash, KeyEqual>::GraphGroups& GraphFilter<K, V, KeyHash, KeyEqual>::edit(
            GraphGroups& graphGroups, const ModType* modType, const std::string& modName) {
        (void)modType;
        (void)modName;

        const std::size_t groups = graphGroups.size();
        for (const GraphId& graphId : graphIds) {
            // An id naming no graph is skipped, as GraphRemove skips one
            if (graphId.iniIndex >= groups) {
                continue;
            }
            Graph* graph = graphGroups.getGraph(graphId.iniIndex, graphId.modObj);
            if (graph != nullptr) {
                filterGraph(*graph, predicate);
            }
        }
        return graphGroups;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::size_t GraphFilter<K, V, KeyHash, KeyEqual>::filterGraph(Graph& graph, const Predicate& predicate) {
        if (!predicate) {
            return 0;
        }

        std::size_t removed = 0;
        for (const auto& entry : graph.sections()) {
            Section* section = entry.second;
            if (section == nullptr) {
                continue;
            }
            auto& parts = section->parts();
            for (std::size_t partInd = 0; partInd < parts.size(); ++partInd) {
                auto* part = dynamic_cast<ContentPart*>(parts[partInd].get());
                if (part == nullptr) {
                    continue;
                }

                // every decision first, against the part as it is; then the removals, back to front
                std::vector<std::size_t> rejected;
                const std::size_t len = part->size();
                for (std::size_t i = 0; i < len; ++i) {
                    if (!predicate(*section, *part, i)) {
                        rejected.push_back(i);
                    }
                }
                for (auto it = rejected.rbegin(); it != rejected.rend(); ++it) {
                    part->removeKVPAt(*it);
                }

                if (!rejected.empty()) {
                    removed += rejected.size();
                    section->refreshCalledSubCommand(partInd);
                }
            }
        }
        return removed;
    }
}

#endif
