#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegDelimitedAdd.h"

#include <algorithm>
#include <memory>


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegDelimitedAdd<K, V, KeyHash, KeyEqual>::RegDelimitedAdd(std::pair<K, V> addition, RegMap delimiterRegs):
        addition(std::move(addition)), delimiterRegs(std::move(delimiterRegs)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<long long> RegDelimitedAdd<K, V, KeyHash, KeyEqual>::getDelimiterInds(const ContentPart& part) const {
        std::vector<long long> result;

        for (const auto& [reg, pred] : delimiterRegs) {
            for (const auto& indVal : part.getValsWithInds(reg)) {
                if (!pred || pred(indVal.second)) {
                    result.push_back(indVal.first);
                }
            }
        }

        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool RegDelimitedAdd<K, V, KeyHash, KeyEqual>::isPathEnd(const CallGraphType& callGraph, ContentPart* part) {
        // A part that makes a 'run =' call continues at its exit node once the call returns; a part
        // that makes none continues at itself -- exitNodeOf picks the right one. Whichever it is,
        // "nothing more executes" is exactly "that node has no outgoing edge": buildCallGraph gives
        // a section's last parts an edge back to every caller's exit, so only the end of a section
        // nobody runs is left without one
        const auto& forwardEdges = callGraph.forwardEdges();
        auto it = forwardEdges.find(callGraph.exitNodeOf(part));
        return it == forwardEdges.end() || it->second.empty();
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegDelimitedAdd<K, V, KeyHash, KeyEqual>::Graph&
    RegDelimitedAdd<K, V, KeyHash, KeyEqual>::edit(Graph& graph, const ModType* modType,
                                                    const std::string& modName, const PartFilter& partFilter,
                                                    bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        (void)modName;
        (void)trackKeys;
        (void)keysToTrack;

        std::unique_ptr<CallGraphType> callGraph = graph.buildCallGraph();

        // The walk starts from the roots and follows 'run =' calls, so a part nobody can reach is
        // never touched; a part reachable more than one way is edited once
        std::unordered_set<ContentPart*> visited;

        auto walk = graph.iterByContentPart();
        while (walk.next()) {
            IterData& iterData = walk.value();
            ContentPart* part = iterData.part;
            if (!visited.insert(part).second) {
                continue;
            }

            std::vector<long long> insertInds = getDelimiterInds(*part);
            if (isPathEnd(*callGraph, part)) {
                insertInds.push_back(static_cast<long long>(part->size()));
            }
            if (insertInds.empty()) {
                continue;
            }

            std::optional<OrderRanges> allowed;
            if (partFilter) {
                allowed = partFilter(iterData, modType, nullptr);
            }

            // Back to front, so an insertion never shifts the indices of the ones still to come
            for (auto it = insertInds.rbegin(); it != insertInds.rend(); ++it) {
                if (allowed.has_value() && !allowed->has(*it)) {
                    continue;
                }
                part->addKVPAt(*it, addition.first, addition.second);
            }
        }

        return graph;
    }
}
