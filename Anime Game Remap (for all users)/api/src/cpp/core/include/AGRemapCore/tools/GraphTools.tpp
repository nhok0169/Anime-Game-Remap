#include "AGRemapCore/tools/GraphTools.h"

#include <deque>


namespace AGRemapCore {

    namespace GraphToolsInternal {
        // Small, header-local helpers -- mirror Python's dict.get(key, default)/set membership
        // without requiring every call site to spell out the iterator-based lookup by hand.

        template <typename Node, typename NodeHash, typename NodeEqual>
        const std::vector<Node>& getEdges(const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& edges, const Node& node) {
            static const std::vector<Node> empty;
            auto it = edges.find(node);
            return (it != edges.end()) ? it->second : empty;
        }

        template <typename Node, typename NodeHash, typename NodeEqual>
        bool getFact(const std::unordered_map<Node, bool, NodeHash, NodeEqual>& facts, const Node& node, bool defaultVal) {
            auto it = facts.find(node);
            return (it != facts.end()) ? it->second : defaultVal;
        }

        template <typename Node, typename NodeHash, typename NodeEqual>
        bool contains(const std::unordered_set<Node, NodeHash, NodeEqual>& nodes, const Node& node) {
            return nodes.find(node) != nodes.end();
        }
    }


    template <typename Node, typename NodeHash, typename NodeEqual>
    std::unordered_set<Node, NodeHash, NodeEqual> GraphTools::getReachableNodes(const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& forwardEdges,
                                                                                 const std::unordered_set<Node, NodeHash, NodeEqual>& rootNodes) {
        std::unordered_set<Node, NodeHash, NodeEqual> reachable(rootNodes.begin(), rootNodes.end());
        std::vector<Node> stack(rootNodes.begin(), rootNodes.end());

        while (!stack.empty()) {
            Node node = std::move(stack.back());
            stack.pop_back();

            for (const Node& succ : GraphToolsInternal::getEdges(forwardEdges, node)) {
                if (reachable.find(succ) == reachable.end()) {
                    reachable.insert(succ);
                    stack.push_back(succ);
                }
            }
        }

        return reachable;
    }

    template <typename Node, typename NodeHash, typename NodeEqual>
    std::unordered_map<Node, bool, NodeHash, NodeEqual> GraphTools::clampFactsToReachable(const std::unordered_map<Node, bool, NodeHash, NodeEqual>& facts,
                                                                                            const std::unordered_set<Node, NodeHash, NodeEqual>& reachableNodes) {
        std::unordered_map<Node, bool, NodeHash, NodeEqual> result;
        result.reserve(facts.size());

        for (const auto& entry : facts) {
            result[entry.first] = entry.second && GraphToolsInternal::contains(reachableNodes, entry.first);
        }

        return result;
    }

    template <typename Node, typename NodeHash, typename NodeEqual>
    std::unordered_map<Node, bool, NodeHash, NodeEqual> GraphTools::runForwardMustFixpoint(const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& forwardEdges,
                                                                                             const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& backwardEdges,
                                                                                             const std::unordered_set<Node, NodeHash, NodeEqual>& rootNodes,
                                                                                             const std::unordered_map<Node, std::pair<bool, bool>, NodeHash, NodeEqual>& localFacts) {
        using EdgeMap = std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>;
        using BoolMap = std::unordered_map<Node, bool, NodeHash, NodeEqual>;
        using NodeSet = std::unordered_set<Node, NodeHash, NodeEqual>;

        NodeSet allNodes;
        for (const auto& entry : forwardEdges) allNodes.insert(entry.first);
        for (const auto& entry : backwardEdges) allNodes.insert(entry.first);
        for (const auto& entry : localFacts) allNodes.insert(entry.first);

        NodeSet roots(rootNodes.begin(), rootNodes.end());
        for (const Node& node : allNodes) {
            if (GraphToolsInternal::getEdges(backwardEdges, node).empty()) {
                roots.insert(node);
            }
        }

        BoolMap entryFact;
        for (const Node& node : allNodes) {
            entryFact[node] = !GraphToolsInternal::contains(roots, node);
        }

        auto computeExit = [&](const Node& node, const BoolMap& entry) -> bool {
            auto factIt = localFacts.find(node);
            if (factIt != localFacts.end()) {
                bool touches = factIt->second.first;
                bool localSatisfied = factIt->second.second;
                return touches ? localSatisfied : GraphToolsInternal::getFact(entry, node, true);
            }
            return GraphToolsInternal::getFact(entry, node, true);
        };

        BoolMap exitFact;
        for (const Node& node : allNodes) {
            exitFact[node] = computeExit(node, entryFact);
        }

        std::deque<Node> worklist(allNodes.begin(), allNodes.end());
        NodeSet queued(allNodes.begin(), allNodes.end());

        while (!worklist.empty()) {
            Node node = std::move(worklist.front());
            worklist.pop_front();
            queued.erase(node);

            if (!GraphToolsInternal::contains(roots, node)) {
                const std::vector<Node>& preds = GraphToolsInternal::getEdges(backwardEdges, node);
                if (!preds.empty()) {
                    bool allSatisfied = true;
                    for (const Node& pred : preds) {
                        if (!GraphToolsInternal::getFact(exitFact, pred, true)) {
                            allSatisfied = false;
                            break;
                        }
                    }
                    entryFact[node] = allSatisfied;
                }
            }

            bool newExit = computeExit(node, entryFact);
            if (newExit != GraphToolsInternal::getFact(exitFact, node, true)) {
                exitFact[node] = newExit;
                for (const Node& succ : GraphToolsInternal::getEdges(forwardEdges, node)) {
                    if (!GraphToolsInternal::contains(queued, succ)) {
                        worklist.push_back(succ);
                        queued.insert(succ);
                    }
                }
            }
        }

        return entryFact;
    }

    template <typename Node, typename NodeHash, typename NodeEqual>
    std::unordered_map<Node, bool, NodeHash, NodeEqual> GraphTools::runBackwardMustFixpoint(const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& forwardEdges,
                                                                                              const std::unordered_map<Node, std::vector<Node>, NodeHash, NodeEqual>& backwardEdges,
                                                                                              const std::unordered_map<Node, bool, NodeHash, NodeEqual>& localFacts) {
        using BoolMap = std::unordered_map<Node, bool, NodeHash, NodeEqual>;
        using NodeSet = std::unordered_set<Node, NodeHash, NodeEqual>;

        NodeSet allNodes;
        for (const auto& entry : forwardEdges) allNodes.insert(entry.first);
        for (const auto& entry : backwardEdges) allNodes.insert(entry.first);
        for (const auto& entry : localFacts) allNodes.insert(entry.first);

        NodeSet terminals;
        for (const Node& node : allNodes) {
            if (GraphToolsInternal::getEdges(forwardEdges, node).empty()) {
                terminals.insert(node);
            }
        }

        auto computeEntry = [&](const Node& node, const BoolMap& exit) -> bool {
            if (GraphToolsInternal::getFact(localFacts, node, false)) {
                return true;
            }
            return GraphToolsInternal::getFact(exit, node, true);
        };

        BoolMap exitFact;
        for (const Node& node : allNodes) {
            exitFact[node] = !GraphToolsInternal::contains(terminals, node);
        }

        BoolMap entryFact;
        for (const Node& node : allNodes) {
            entryFact[node] = computeEntry(node, exitFact);
        }

        std::deque<Node> worklist(allNodes.begin(), allNodes.end());
        NodeSet queued(allNodes.begin(), allNodes.end());

        while (!worklist.empty()) {
            Node node = std::move(worklist.front());
            worklist.pop_front();
            queued.erase(node);

            if (!GraphToolsInternal::contains(terminals, node)) {
                const std::vector<Node>& succs = GraphToolsInternal::getEdges(forwardEdges, node);
                if (!succs.empty()) {
                    bool allGuaranteed = true;
                    for (const Node& succ : succs) {
                        if (!GraphToolsInternal::getFact(entryFact, succ, true)) {
                            allGuaranteed = false;
                            break;
                        }
                    }
                    exitFact[node] = allGuaranteed;
                }
            }

            bool newEntry = computeEntry(node, exitFact);
            if (newEntry != GraphToolsInternal::getFact(entryFact, node, true)) {
                entryFact[node] = newEntry;
                for (const Node& pred : GraphToolsInternal::getEdges(backwardEdges, node)) {
                    if (!GraphToolsInternal::contains(queued, pred)) {
                        worklist.push_back(pred);
                        queued.insert(pred);
                    }
                }
            }
        }

        return exitFact;
    }
}
