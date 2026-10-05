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

#ifndef AGRemapCore_GraphInherit_TPP
#define AGRemapCore_GraphInherit_TPP

#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupPartEdits.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAdd.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphInherit<K, V, KeyHash, KeyEqual>::GraphInherit(GraphId src, GraphId dst, K reg, bool latest, PartFilter partFilter, Adder adder):
        src(std::move(src)), dst(std::move(dst)), reg(std::move(reg)), latest(latest), partFilter(std::move(partFilter)),
        adder(std::move(adder)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphInherit<K, V, KeyHash, KeyEqual>::GraphGroups& GraphInherit<K, V, KeyHash, KeyEqual>::edit(GraphGroups& graphGroups, const ModType* modType,
                                                                                                             const std::string& modName) {
        return editImpl(graphGroups, nullptr, modType, modName);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphInherit<K, V, KeyHash, KeyEqual>::GraphGroups& GraphInherit<K, V, KeyHash, KeyEqual>::editFromIni(GraphGroups& graphGroups, IniFile* ini,
                                                                                                                    const ModType* modType,
                                                                                                                    const std::string& modName) {
        return editImpl(graphGroups, ini, modType, modName);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GraphInherit<K, V, KeyHash, KeyEqual>::addWithEdit(GraphGroups& graphGroups, Graph& srcGraph, const KVPs& kvps, IniFile* ini,
                                                            const ModType* modType, const std::string& modName) {
        AddEdit addEdit = adder(srcGraph, kvps, ini, modType, modName);

        // The adder's edit runs exactly as a GraphGroupEdit would run it on this one graph, so
        // what a register edit or a graph edit does with its key filter here is what it does
        // everywhere else. The shared_ptr in 'addEdit' keeps the edit alive for the whole call;
        // the adapters only borrow it.
        using Owner = GraphGroupEdit<K, V, KeyHash, KeyEqual>;
        std::unique_ptr<typename Owner::PartEdit> partEdit;

        if (auto* regEdit = std::get_if<std::shared_ptr<RegEdit>>(&addEdit); regEdit != nullptr && *regEdit != nullptr) {
            partEdit = std::make_unique<RegPartEdit<K, V, KeyHash, KeyEqual>>(regEdit->get());
        } else if (auto* graphEdit = std::get_if<std::shared_ptr<GraphEdit>>(&addEdit); graphEdit != nullptr && *graphEdit != nullptr) {
            partEdit = std::make_unique<GraphPartEdit<K, V, KeyHash, KeyEqual>>(graphEdit->get());
        }

        // monostate (or a null edit): the adder did the insertion itself
        if (partEdit == nullptr) {
            return;
        }

        std::vector<typename Owner::PartFilter> keyFilters;
        if (partFilter) {
            keyFilters.push_back(partFilter);
        }

        Graph* edited = Owner::editSectionGraph(srcGraph, {partEdit.get()}, ini, modType, modName, keyFilters);
        if (edited != nullptr && edited != &srcGraph) {
            Base::addGraph(graphGroups, src, edited);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphInherit<K, V, KeyHash, KeyEqual>::GraphGroups& GraphInherit<K, V, KeyHash, KeyEqual>::editImpl(GraphGroups& graphGroups, IniFile* ini,
                                                                                                                 const ModType* modType,
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
        KVPs kvps;

        for (const std::string& rootName : dstGraph->roots()) {
            kvps.emplace_back(reg, runConfig.valOfSectionName(rootName));
        }

        if (kvps.empty()) {
            return graphGroups;
        }

        if (adder) {
            addWithEdit(graphGroups, *srcGraph, kvps, ini, modType, modName);
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
            OrderRanges partRanges = partFilter(iterData, modType, ini);
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
