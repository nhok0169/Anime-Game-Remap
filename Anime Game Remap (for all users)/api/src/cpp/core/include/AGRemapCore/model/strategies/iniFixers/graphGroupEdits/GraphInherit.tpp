#ifndef AGRemapCore_GraphInherit_TPP
#define AGRemapCore_GraphInherit_TPP

#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAdd.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphInherit<K, V, KeyHash, KeyEqual>::GraphInherit(GraphId src, GraphId dst, K reg, bool latest, PartFilter partFilter):
        src(std::move(src)), dst(std::move(dst)), reg(std::move(reg)), latest(latest), partFilter(std::move(partFilter)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphInherit<K, V, KeyHash, KeyEqual>::GraphGroups& GraphInherit<K, V, KeyHash, KeyEqual>::edit(GraphGroups& graphGroups, const ModType* modType,
                                                                                                             const std::string& modName) {
        // errorOnNotFound = false on both: a missing source or destination graph is this class's
        // documented "do nothing" case, not an error -- see its own top-level note.
        Graph* srcGraph = Base::getGraph(graphGroups, src, false);
        Graph* dstGraph = Base::getGraph(graphGroups, dst, false);

        if (srcGraph == nullptr || dstGraph == nullptr) {
            return graphGroups;
        }

        // The KVP values are `run =`-style references to another section, so they're built the
        // same way every other section reference in this codebase is -- through the graph's own
        // run configuration, rather than assuming V is constructible from a std::string.
        const IfTemplateRunConfig<K, V>& runConfig = srcGraph->runConfig();
        std::vector<std::pair<K, V>> kvps;

        for (const std::string& rootName : dstGraph->roots()) {
            kvps.emplace_back(reg, runConfig.valOfSectionName(rootName));
        }

        if (kvps.empty()) {
            return graphGroups;
        }

        // No filter -- insert straight to the very front/back of every root section of 'src'
        if (!partFilter) {
            for (auto* section : srcGraph->getRootSections()) {
                if (latest) {
                    section->addKVPsToBack(kvps);
                } else {
                    section->addKVPsToFront(kvps);
                }

                section->rebuild();
            }

            return graphGroups;
        }

        // Filter given -- insert at the earliest/latest valid index of every IfContentPart the
        // filter accepts
        RegAdd<K, V, KeyHash, KeyEqual> regAdd(kvps, latest);
        std::unordered_set<typename Base::Graph::Section*> touchedSections;

        // Generator is a single-pass, move-only coroutine type with no begin()/end() -- it is
        // driven with next()/value(), not a range-for.
        auto parts = srcGraph->iterByContentPart();
        while (parts.next()) {
            IterData& iterData = parts.value();
            OrderRanges partRanges = partFilter(iterData, modType, nullptr);
            if (partRanges.isEmpty()) {
                continue;
            }

            regAdd.edit(*iterData.part, iterData.sectionName, modType, modName, &partRanges);
            touchedSections.insert(iterData.section);
        }

        for (auto* section : touchedSections) {
            section->rebuild();
        }

        return graphGroups;
    }
}

#endif
