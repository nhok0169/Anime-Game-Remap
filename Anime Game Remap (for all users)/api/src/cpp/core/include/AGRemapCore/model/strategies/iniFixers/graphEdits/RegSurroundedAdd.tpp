#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.h"

#include <algorithm>
#include <memory>


namespace AGRemapCore {

    namespace RegSurroundedAddInternal {
        // Mirrors Python's dict.get(key, default) -- used throughout for the various
        // register/node fact maps, matching the pure-Python original's own repeated use of
        // Dict.get.
        template <typename M, typename Key>
        bool getFact(const M& facts, const Key& key, bool defaultVal) {
            auto it = facts.find(key);
            return (it != facts.end()) ? it->second : defaultVal;
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::RegSurroundedAdd(std::pair<K, V> addition, RegMap beforeRegs, RegMap afterRegs, bool latest):
        addition(std::move(addition)), beforeRegs(std::move(beforeRegs)), afterRegs(std::move(afterRegs)), latest(latest) {

        _beforeFilters = buildKeyFilters(this->beforeRegs);
        _afterFilters = buildKeyFilters(this->afterRegs);

        for (const auto& [reg, pred] : this->beforeRegs) {
            _trackedKeys.insert(reg);
        }
        for (const auto& [reg, pred] : this->afterRegs) {
            _trackedKeys.insert(reg);
        }
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unordered_map<K, typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::Colouring::Filter, KeyHash, KeyEqual>
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::buildKeyFilters(const RegMap& regs) {
        std::unordered_map<K, typename Colouring::Filter, KeyHash, KeyEqual> result;

        for (const auto& [reg, pred] : regs) {
            // A register whose predicate is empty (Python's None) is left out on purpose --
            // accepting "any value" for a register is already handled by simply checking the
            // register's existence (see getSatisfiedRange).
            if (!pred) {
                continue;
            }
            result[reg] = [pred](std::optional<long long>, const V& val) { return pred(val); };
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::OrderRanges
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::getSatisfiedRange(Colouring& colouring, const KeySet& regNames,
                                                                  const std::unordered_map<K, typename Colouring::Filter, KeyHash, KeyEqual>& filters,
                                                                  bool includeKeyDefs) {
        if (regNames.empty()) {
            return OrderRanges::createFull();
        }

        std::unordered_map<K, bool, KeyHash, KeyEqual> keysExists;
        for (const K& reg : regNames) {
            keysExists[reg] = true;
        }

        return colouring.getRanges(keysExists, filters, true, true, true, includeKeyDefs);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool RegSurroundedAdd<K, V, KeyHash, KeyEqual>::keysExistSomewhere(Graph& graph, const KeySet& keys) {
        KeySet remaining = keys;
        if (remaining.empty()) {
            return true;
        }

        auto walk = graph.iterByContentPart();
        while (walk.next()) {
            IterData& iterData = walk.value();
            ContentPart* part = iterData.part;

            for (auto it = remaining.begin(); it != remaining.end();) {
                if (part->contains(*it)) {
                    it = remaining.erase(it);
                } else {
                    ++it;
                }
            }

            if (remaining.empty()) {
                return true;
            }
        }

        return false;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    long long RegSurroundedAdd<K, V, KeyHash, KeyEqual>::pickInsertInd(const OrderRanges& validRange, const ContentPart& part) const {
        const auto& ranges = validRange.ranges;

        if (latest) {
            std::optional<long long> insertInd = ranges.back().second;
            return insertInd.has_value() ? (*insertInd - 1) : static_cast<long long>(part.size());
        }

        std::optional<long long> insertInd = ranges.front().first;
        return insertInd.has_value() ? *insertInd : 0;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::OrderRanges
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::getForwardValidRangeForPart(const ContentPart& part, const K& runKey,
                                                                            const std::unordered_map<K, bool, KeyHash, KeyEqual>& beforeEntryFacts,
                                                                            const std::unordered_map<K, bool, KeyHash, KeyEqual>& beforeReturnFacts) const {
        using RegSurroundedAddInternal::getFact;

        if (beforeRegs.empty()) {
            return OrderRanges::createFull();
        }

        std::optional<long long> lastCallInd;
        for (const auto& indVal : part.getValsWithInds(runKey)) {
            if (!lastCallInd.has_value() || indVal.first > *lastCallInd) {
                lastCallInd = indVal.first;
            }
        }

        std::vector<OrderRanges> perRegRanges;
        for (const auto& [reg, pred] : beforeRegs) {
            std::unordered_map<K, typename Colouring::Filter, KeyHash, KeyEqual> filters;
            auto filterIt = _beforeFilters.find(reg);
            if (filterIt != _beforeFilters.end()) {
                filters[reg] = filterIt->second;
            }

            Colouring localColouring;
            KeySet targetKeys{reg};
            localColouring.updateColouring(part, targetKeys);

            if (!localColouring.contains(reg)) {
                OrderRanges regRange = getFact(beforeEntryFacts, reg, false) ? OrderRanges::createFull() : OrderRanges::createEmpty();
                if (lastCallInd.has_value() && getFact(beforeReturnFacts, reg, false)) {
                    regRange = regRange.unionWith({OrderRanges({{*lastCallInd + 1, std::nullopt}}, true)});
                }
                perRegRanges.push_back(regRange);
                continue;
            }

            OrderRanges localRange = getSatisfiedRange(localColouring, KeySet{reg}, filters, false);
            if (getFact(beforeEntryFacts, reg, false)) {
                std::optional<long long> firstInd;
                for (const auto& indVal : part.getValsWithInds(reg)) {
                    if (!firstInd.has_value() || indVal.first < *firstInd) {
                        firstInd = indVal.first;
                    }
                }
                localRange = localRange.unionWith({OrderRanges({{0, *firstInd + 1}}, true)});
            }

            perRegRanges.push_back(localRange);
        }

        return perRegRanges[0].intersect(std::vector<OrderRanges>(perRegRanges.begin() + 1, perRegRanges.end()));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::OrderRanges
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::getBackwardValidRangeForPart(const ContentPart& part, const K& runKey,
                                                                             const std::unordered_map<K, bool, KeyHash, KeyEqual>& afterExitFacts,
                                                                             const std::unordered_map<K, bool, KeyHash, KeyEqual>& afterReturnFacts) const {
        using RegSurroundedAddInternal::getFact;

        if (afterRegs.empty()) {
            return OrderRanges::createFull();
        }

        std::optional<long long> lastCallInd;
        for (const auto& indVal : part.getValsWithInds(runKey)) {
            if (!lastCallInd.has_value() || indVal.first > *lastCallInd) {
                lastCallInd = indVal.first;
            }
        }

        std::vector<OrderRanges> perRegRanges;
        for (const auto& [reg, pred] : afterRegs) {
            std::vector<typename OrderRanges::Range> acceptedRanges;
            for (const auto& indVal : part.getValsWithInds(reg)) {
                if (!pred || pred(indVal.second)) {
                    acceptedRanges.push_back({std::nullopt, indVal.first + 1});
                }
            }
            OrderRanges regRange = acceptedRanges.empty() ? OrderRanges::createEmpty() : OrderRanges(acceptedRanges);

            if (!lastCallInd.has_value()) {
                if (getFact(afterExitFacts, reg, false)) {
                    regRange = regRange.unionWith({OrderRanges::createFull()});
                }
            } else {
                if (getFact(afterExitFacts, reg, false)) {
                    regRange = regRange.unionWith({OrderRanges({{0, *lastCallInd + 1}}, true)});
                }
                if (getFact(afterReturnFacts, reg, false)) {
                    regRange = regRange.unionWith({OrderRanges({{*lastCallInd + 1, std::nullopt}}, true)});
                }
            }

            perRegRanges.push_back(regRange);
        }

        return perRegRanges[0].intersect(std::vector<OrderRanges>(perRegRanges.begin() + 1, perRegRanges.end()));
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::OrderRanges
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::getValidRangeForPart(const ContentPart& part, const K& runKey,
                                                                     const std::unordered_map<K, bool, KeyHash, KeyEqual>& beforeEntryFacts,
                                                                     const std::unordered_map<K, bool, KeyHash, KeyEqual>& beforeReturnFacts,
                                                                     const std::unordered_map<K, bool, KeyHash, KeyEqual>& afterExitFacts,
                                                                     const std::unordered_map<K, bool, KeyHash, KeyEqual>& afterReturnFacts) const {
        std::unordered_map<K, bool, KeyHash, KeyEqual> emptyFacts;

        OrderRanges localOnlyRange = getForwardValidRangeForPart(part, runKey, beforeEntryFacts, emptyFacts).intersect(
            {getBackwardValidRangeForPart(part, runKey, emptyFacts, emptyFacts)});
        if (!localOnlyRange.isEmpty()) {
            return localOnlyRange;
        }

        OrderRanges mustComeAfterRange = getForwardValidRangeForPart(part, runKey, beforeEntryFacts, beforeReturnFacts);
        OrderRanges mustComeBeforeRange = getBackwardValidRangeForPart(part, runKey, afterExitFacts, afterReturnFacts);
        return mustComeAfterRange.intersect({mustComeBeforeRange});
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::OrderRanges
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::getClippedValidRangeForPart(const IterData& iterData, const ModType* modType, const K& runKey, const PartFilter& partFilter,
                                                                            const std::unordered_map<K, std::unordered_map<Node, bool, NodeHash>, KeyHash, KeyEqual>& beforeEntryFacts,
                                                                            const std::unordered_map<K, std::unordered_map<Node, bool, NodeHash>, KeyHash, KeyEqual>& afterExitFacts) const {
        using RegSurroundedAddInternal::getFact;

        ContentPart* part = iterData.part;
        Node partNode{part, false};
        Node exitNode{part, part->contains(runKey)};

        std::unordered_map<K, bool, KeyHash, KeyEqual> partBeforeFacts;
        std::unordered_map<K, bool, KeyHash, KeyEqual> partBeforeReturnFacts;
        for (const auto& [reg, facts] : beforeEntryFacts) {
            partBeforeFacts[reg] = getFact(facts, partNode, false);
            partBeforeReturnFacts[reg] = getFact(facts, exitNode, false);
        }

        std::unordered_map<K, bool, KeyHash, KeyEqual> partAfterFacts;
        std::unordered_map<K, bool, KeyHash, KeyEqual> partAfterReturnFacts;
        for (const auto& [reg, facts] : afterExitFacts) {
            partAfterFacts[reg] = getFact(facts, partNode, false);
            partAfterReturnFacts[reg] = getFact(facts, exitNode, false);
        }

        OrderRanges validRange = getValidRangeForPart(*part, runKey, partBeforeFacts, partBeforeReturnFacts, partAfterFacts, partAfterReturnFacts);
        if (validRange.isEmpty()) {
            return validRange;
        }

        std::vector<OrderRanges> bounds;
        bounds.push_back(OrderRanges({{0, static_cast<long long>(part->size()) + 1}}, true));
        if (partFilter) {
            bounds.push_back(partFilter(iterData, modType, nullptr));
        }

        return validRange.intersect(bounds);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<bool, bool> RegSurroundedAdd<K, V, KeyHash, KeyEqual>::computeLocalForwardFact(const ContentPart& part, const K& reg, const Predicate& pred) {
        Colouring colouring;
        KeySet targetKeys{reg};
        colouring.updateColouring(part, targetKeys);
        if (!colouring.contains(reg)) {
            return {false, false};
        }

        std::unordered_map<K, typename Colouring::Filter, KeyHash, KeyEqual> filters;
        if (pred) {
            filters[reg] = [pred](std::optional<long long>, const V& val) { return pred(val); };
        }

        std::unordered_map<K, bool, KeyHash, KeyEqual> keysExists{{reg, true}};
        OrderRanges satisfiedRange = colouring.getRanges(keysExists, filters, true, true, true, true);
        return {true, satisfiedRange.has(static_cast<long long>(part.size()))};
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool RegSurroundedAdd<K, V, KeyHash, KeyEqual>::computeLocalBackwardFact(const ContentPart& part, const K& reg, const Predicate& pred) {
        for (const auto& indVal : part.getValsWithInds(reg)) {
            if (!pred || pred(indVal.second)) {
                return true;
            }
        }
        return false;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<std::unordered_map<K, std::unordered_map<typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::Node, bool, typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::NodeHash>, KeyHash, KeyEqual>,
              std::unordered_map<K, std::unordered_map<typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::Node, bool, typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::NodeHash>, KeyHash, KeyEqual>>
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::computeKeyFacts(Graph& graph) const {
        using NodeFactMap = std::unordered_map<Node, bool, NodeHash>;
        using RegNodeFactMap = std::unordered_map<K, NodeFactMap, KeyHash, KeyEqual>;

        if (beforeRegs.empty() && afterRegs.empty()) {
            return {RegNodeFactMap{}, RegNodeFactMap{}};
        }

        std::unique_ptr<CallGraphType> callGraph = graph.buildCallGraph();

        std::unordered_set<Node, NodeHash> rootNodes;
        for (ContentPart* part : callGraph->rootNodes()) {
            rootNodes.insert(Node{part, false});
        }

        std::unordered_set<Node, NodeHash> reachableNodes = GraphTools::getReachableNodes<Node, NodeHash>(callGraph->forwardEdges(), rootNodes);

        RegNodeFactMap beforeEntryFacts;
        for (const auto& [reg, pred] : beforeRegs) {
            std::unordered_map<Node, std::pair<bool, bool>, NodeHash> localFacts;
            for (ContentPart* part : callGraph->parts()) {
                localFacts[Node{part, false}] = computeLocalForwardFact(*part, reg, pred);
            }
            NodeFactMap rawFacts = GraphTools::runForwardMustFixpoint<Node, NodeHash>(callGraph->forwardEdges(), callGraph->backwardEdges(), rootNodes, localFacts);
            beforeEntryFacts[reg] = GraphTools::clampFactsToReachable<Node, NodeHash>(rawFacts, reachableNodes);
        }

        RegNodeFactMap afterExitFacts;
        for (const auto& [reg, pred] : afterRegs) {
            NodeFactMap localFacts;
            for (ContentPart* part : callGraph->parts()) {
                localFacts[Node{part, false}] = computeLocalBackwardFact(*part, reg, pred);
            }
            NodeFactMap rawFacts = GraphTools::runBackwardMustFixpoint<Node, NodeHash>(callGraph->forwardEdges(), callGraph->backwardEdges(), localFacts);
            afterExitFacts[reg] = GraphTools::clampFactsToReachable<Node, NodeHash>(rawFacts, reachableNodes);
        }

        return {beforeEntryFacts, afterExitFacts};
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::Graph&
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::editEarliest(Graph& graph, const ModType* modType, const PartFilter& partFilter) {
        using RegSurroundedAddInternal::getFact;

        std::unordered_map<ContentPart*, std::vector<ContentPart*>> predecessors = graph.buildPartPredecessorGraph();
        auto keyFacts = computeKeyFacts(graph);
        const auto& beforeEntryFacts = keyFacts.first;
        const auto& afterExitFacts = keyFacts.second;
        const K& runKey = graph.runConfig().runKey;
        std::unordered_map<ContentPart*, bool> claimed;

        auto walk = graph.iterByContentPart(1, true, _trackedKeys);
        while (walk.next()) {
            IterData& iterData = walk.value();
            ContentPart* part = iterData.part;

            bool isClaimed = false;
            auto predIt = predecessors.find(part);
            if (predIt != predecessors.end()) {
                for (ContentPart* pred : predIt->second) {
                    if (getFact(claimed, pred, false)) {
                        isClaimed = true;
                        break;
                    }
                }
            }

            if (!isClaimed) {
                OrderRanges validRange = getClippedValidRangeForPart(iterData, modType, runKey, partFilter, beforeEntryFacts, afterExitFacts);
                if (!validRange.isEmpty()) {
                    part->addKVPAt(pickInsertInd(validRange, *part), addition.first, addition.second);
                    isClaimed = true;
                }
            }

            claimed[part] = isClaimed;
        }

        return graph;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::Graph&
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::editLatest(Graph& graph, const ModType* modType, const PartFilter& partFilter) {
        using RegSurroundedAddInternal::getFact;

        std::unordered_map<ContentPart*, std::vector<ContentPart*>> predecessors = graph.buildPartPredecessorGraph();
        std::unordered_map<ContentPart*, std::vector<ContentPart*>> successors;
        for (const auto& [part, preds] : predecessors) {
            for (ContentPart* pred : preds) {
                successors[pred].push_back(part);
            }
        }

        auto keyFacts = computeKeyFacts(graph);
        const auto& beforeEntryFacts = keyFacts.first;
        const auto& afterExitFacts = keyFacts.second;
        const K& runKey = graph.runConfig().runKey;

        std::unordered_map<ContentPart*, std::optional<std::pair<ContentPart*, long long>>> candidates;
        std::vector<ContentPart*> visitOrder;

        auto walk = graph.iterByContentPart(1, true, _trackedKeys);
        while (walk.next()) {
            IterData& iterData = walk.value();
            ContentPart* part = iterData.part;

            OrderRanges validRange = getClippedValidRangeForPart(iterData, modType, runKey, partFilter, beforeEntryFacts, afterExitFacts);
            if (validRange.isEmpty()) {
                candidates[part] = std::nullopt;
            } else {
                candidates[part] = std::make_pair(part, pickInsertInd(validRange, *part));
            }
            visitOrder.push_back(part);
        }

        std::unordered_map<ContentPart*, bool> claimed;
        for (auto it = visitOrder.rbegin(); it != visitOrder.rend(); ++it) {
            ContentPart* part = *it;

            bool isClaimed = false;
            auto succIt = successors.find(part);
            if (succIt != successors.end()) {
                for (ContentPart* succ : succIt->second) {
                    if (getFact(claimed, succ, false)) {
                        isClaimed = true;
                        break;
                    }
                }
            }

            if (!isClaimed && candidates[part].has_value()) {
                ContentPart* candidatePart = candidates[part]->first;
                long long candidateInd = candidates[part]->second;
                candidatePart->addKVPAt(candidateInd, addition.first, addition.second);
                isClaimed = true;
            }

            claimed[part] = isClaimed;
        }

        return graph;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegSurroundedAdd<K, V, KeyHash, KeyEqual>::Graph&
    RegSurroundedAdd<K, V, KeyHash, KeyEqual>::edit(Graph& graph, const ModType* modType,
                                                     const std::string& modName, const PartFilter& partFilter,
                                                     bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        // 'modName'/'trackKeys'/'keysToTrack' are the caller's defaults, handed down by
        // BaseIniGraphEdit's contract (GraphGroupEdit passes its own). This edit builds its own
        // colourings from its own beforeRegs/afterRegs, so it has no use for them -- they are
        // accepted only so the shared call convention keeps working.
        (void)modName;
        (void)trackKeys;
        (void)keysToTrack;

        if (!afterRegs.empty()) {
            KeySet afterKeys;
            for (const auto& [reg, pred] : afterRegs) {
                afterKeys.insert(reg);
            }
            if (!keysExistSomewhere(graph, afterKeys)) {
                return graph;
            }
        }

        if (latest) {
            return editLatest(graph, modType, partFilter);
        }
        return editEarliest(graph, modType, partFilter);
    }
}
