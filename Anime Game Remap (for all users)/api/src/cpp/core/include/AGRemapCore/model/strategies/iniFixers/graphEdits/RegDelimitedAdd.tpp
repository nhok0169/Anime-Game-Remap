#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegDelimitedAdd.h"

#include <algorithm>
#include <memory>


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegDelimitedAdd<K, V, KeyHash, KeyEqual>::RegDelimitedAdd(Additions additions, RegMap delimiterRegs,
                                                                 bool pathEndOnlyWhenUndelimited):
        additions(std::move(additions)), delimiterRegs(std::move(delimiterRegs)),
        pathEndOnlyWhenUndelimited(pathEndOnlyWhenUndelimited) {}

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
    void RegDelimitedAdd<K, V, KeyHash, KeyEqual>::addAdditionsAt(ContentPart& part, long long index) const {
        // Front to back at index, index+1, ...: each insertion only shifts what comes after it, so
        // the list ends up in its own order and nothing already in the part moves relative to it
        long long ind = index;
        for (const auto& [key, val] : additions) {
            part.addKVPAt(ind, key, val);
            ++ind;
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegDelimitedAdd<K, V, KeyHash, KeyEqual>::Graph&
    RegDelimitedAdd<K, V, KeyHash, KeyEqual>::edit(Graph& graph, const ModType* modType,
                                                    const std::string& modName, const PartFilter& partFilter,
                                                    bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        (void)modName;
        (void)trackKeys;
        (void)keysToTrack;

        if (additions.empty()) {
            return graph;
        }

        std::unique_ptr<CallGraphType> callGraph = graph.buildCallGraph();

        // Which sections delimit, directly or through a section they run. Only needed for
        // pathEndOnlyWhenUndelimited, and built BEFORE the loop below starts inserting so it
        // describes the graph as it arrived. A part can end a path having delimited nothing itself
        // while the section it ran delimited before returning, which is the whole reason the callee
        // has to be consulted rather than just this part.
        std::unordered_map<std::string, bool> sectionDelimits;
        std::unordered_map<std::string, std::unordered_set<std::string>> sectionCalls;

        if (pathEndOnlyWhenUndelimited) {
            auto scan = graph.iterByContentPart();
            while (scan.next()) {
                IterData& scanData = scan.value();
                if (!getDelimiterInds(*scanData.part).empty()) {
                    sectionDelimits[scanData.sectionName] = true;
                }

                for (const V& runVal : scanData.part->getVals(graph.runConfig().runKey)) {
                    sectionCalls[scanData.sectionName].insert(graph.runConfig().sectionNameOf(runVal));
                }
            }

            // Transitive closure. One section per mod object, so iterating to a fixed point costs
            // nothing and handles a call cycle without special-casing it.
            bool changed = true;
            while (changed) {
                changed = false;
                for (const auto& callEntry : sectionCalls) {
                    auto delimitsIt = sectionDelimits.find(callEntry.first);
                    if (delimitsIt != sectionDelimits.end() && delimitsIt->second) {
                        continue;
                    }

                    for (const std::string& callee : callEntry.second) {
                        auto calleeIt = sectionDelimits.find(callee);
                        if (calleeIt != sectionDelimits.end() && calleeIt->second) {
                            sectionDelimits[callEntry.first] = true;
                            changed = true;
                            break;
                        }
                    }
                }
            }
        }

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
                bool addAtEnd = true;

                if (pathEndOnlyWhenUndelimited) {
                    // This part delimits, or something it ran did: the path has already had its
                    // additions placed before those delimiters, and one more after the last of them
                    // would be a surplus call.
                    addAtEnd = insertInds.empty();

                    if (addAtEnd) {
                        auto callsIt = sectionCalls.find(iterData.sectionName);
                        if (callsIt != sectionCalls.end()) {
                            for (const std::string& callee : callsIt->second) {
                                auto calleeIt = sectionDelimits.find(callee);
                                if (calleeIt != sectionDelimits.end() && calleeIt->second) {
                                    addAtEnd = false;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (addAtEnd) {
                    insertInds.push_back(static_cast<long long>(part->size()));
                }
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
                addAdditionsAt(*part, *it);
            }
        }

        return graph;
    }
}
