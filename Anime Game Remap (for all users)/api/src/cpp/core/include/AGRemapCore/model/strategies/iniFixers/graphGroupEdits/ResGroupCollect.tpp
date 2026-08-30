#ifndef AGRemapCore_ResGroupCollect_TPP
#define AGRemapCore_ResGroupCollect_TPP

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"


namespace AGRemapCore {

    // ---------------------------------------------------------------------------------------
    // ResRootLocation / ResGroup
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool ResGroupCollect<K, V, KeyHash, KeyEqual>::ResRootLocation::operator<(const ResRootLocation& other) const {
        if (resModObj.iniIndex != other.resModObj.iniIndex) return resModObj.iniIndex < other.resModObj.iniIndex;
        if (resModObj.modObj != other.resModObj.modObj) return resModObj.modObj < other.resModObj.modObj;
        if (srcModObj.iniIndex != other.srcModObj.iniIndex) return srcModObj.iniIndex < other.srcModObj.iniIndex;
        if (srcModObj.modObj != other.srcModObj.modObj) return srcModObj.modObj < other.srcModObj.modObj;
        if (sectionName != other.sectionName) return sectionName < other.sectionName;
        if (partId != other.partId) return partId < other.partId;
        return orderInd < other.orderInd;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool ResGroupCollect<K, V, KeyHash, KeyEqual>::ResRootLocation::operator==(const ResRootLocation& other) const {
        return resModObj == other.resModObj && srcModObj == other.srcModObj && sectionName == other.sectionName &&
               partId == other.partId && orderInd == other.orderInd;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool ResGroupCollect<K, V, KeyHash, KeyEqual>::ResGroup::isMissing(const std::unordered_set<GraphId, GraphIdHash>& collected) const {
        for (const GraphId& resType : collected) {
            if (entries.find(resType) == entries.end()) {
                return true;
            }
        }

        return false;
    }


    // ---------------------------------------------------------------------------------------
    // construction / configuration
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    ResGroupCollect<K, V, KeyHash, KeyEqual>::ResGroupCollect(std::vector<std::string> resGroupTypes, ByGraph<ByGraph<K>> srcRegs,
                                                               ByGraph<tsl::ordered_map<std::string, ResEdit*>> resEdits,
                                                               tsl::ordered_map<std::string, GroupedResBuilder*> groupedResBuilders,
                                                               std::function<V(const std::string&)> valOfSectionName,
                                                               long long id):
        srcRegs(std::move(srcRegs)), resEdits(std::move(resEdits)), groupedResBuilders(std::move(groupedResBuilders)),
        id(id), nullValue(IniKeywords::Null), valOfSectionName(std::move(valOfSectionName)) {
        setResGroupTypes(std::move(resGroupTypes));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<std::string>& ResGroupCollect<K, V, KeyHash, KeyEqual>::resGroupTypes() const {
        return resGroupTypes_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::setResGroupTypes(std::vector<std::string> newResGroupTypes) {
        std::vector<std::string> result;
        std::unordered_set<std::string> seen;

        for (std::string& resGroupType : newResGroupTypes) {
            if (!seen.insert(resGroupType).second) {
                continue;
            }

            result.push_back(std::move(resGroupType));
        }

        resGroupTypes_ = std::move(result);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::clear() {
        resCalls.clear();

        for (const auto& resTypeEntry : resEdits) {
            for (const auto& groupTypeEntry : resTypeEntry.second) {
                if (groupTypeEntry.second != nullptr) {
                    groupTypeEntry.second->clear();
                }
            }
        }
    }


    // ---------------------------------------------------------------------------------------
    // queries
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    Z3Predicate ResGroupCollect<K, V, KeyHash, KeyEqual>::combineQueries(const Z3Predicate& a, const Z3Predicate& b,
                                                                         Z3Context* targetZ3Ctx) {
        if (targetZ3Ctx == nullptr) {
            return a & b;
        }

        // belongsTo is a raw pointer comparison; reparent is a full round trip. Checking first is
        // what keeps the common same-context case free -- see this method's own note.
        std::optional<Z3Predicate> reparentedA;
        std::optional<Z3Predicate> reparentedB;

        if (!a.belongsTo(*targetZ3Ctx)) {
            reparentedA = IfPredPart::reparent(a, *targetZ3Ctx);
            if (!reparentedA.has_value()) {
                throw std::invalid_argument("Failed to reparent a query into the target Z3Context.");
            }
        }

        if (!b.belongsTo(*targetZ3Ctx)) {
            reparentedB = IfPredPart::reparent(b, *targetZ3Ctx);
            if (!reparentedB.has_value()) {
                throw std::invalid_argument("Failed to reparent a query into the target Z3Context.");
            }
        }

        const Z3Predicate& left = reparentedA.has_value() ? *reparentedA : a;
        const Z3Predicate& right = reparentedB.has_value() ? *reparentedB : b;
        return left & right;
    }


    // ---------------------------------------------------------------------------------------
    // collect
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::collectFromGraphGroup(GraphGroups& graphGroups, const GraphId& resModObj,
                                                                          const GraphId& srcModObj, const K& srcReg) {
        Graph* graph = Base::getGraph(graphGroups, srcModObj, false);
        if (graph == nullptr) {
            return;
        }

        const PartPredicate* partPredicate = nullptr;
        auto partPredicateOuter = partPredicates.find(resModObj);
        if (partPredicateOuter != partPredicates.end()) {
            auto it = partPredicateOuter->second.find(srcModObj);
            if (it != partPredicateOuter->second.end() && it->second) {
                partPredicate = &(it->second);
            }
        }

        const ResPredicate* resPredicate = nullptr;
        auto resPredicateOuter = resPredicates.find(resModObj);
        if (resPredicateOuter != resPredicates.end()) {
            auto it = resPredicateOuter->second.find(srcModObj);
            if (it != resPredicateOuter->second.end() && it->second) {
                resPredicate = &(it->second);
            }
        }

        std::optional<std::unordered_set<K, KeyHash, KeyEqual>> currentKeysToTrack;
        auto keysToTrackOuter = keysToTrack.find(resModObj);
        if (keysToTrackOuter != keysToTrack.end()) {
            auto it = keysToTrackOuter->second.find(srcModObj);
            if (it != keysToTrackOuter->second.end()) {
                currentKeysToTrack = it->second;
            }
        }

        bool currentTrackKeys = trackKeysGlobal;
        if (!trackKeysIsGlobal) {
            currentTrackKeys = false;
            auto trackKeysOuter = trackKeys.find(resModObj);
            if (trackKeysOuter != trackKeys.end()) {
                auto it = trackKeysOuter->second.find(srcModObj);
                if (it != trackKeysOuter->second.end()) {
                    currentTrackKeys = it->second;
                }
            }
        }

        auto parts = graph->iterByQuery({}, false, 1, currentTrackKeys, currentKeysToTrack);
        while (parts.next()) {
            IterQueryData& iterData = parts.value();
            auto* part = iterData.part;

            std::optional<typename ContentPart::RangeSpec> partRanges;
            if (partPredicate != nullptr) {
                partRanges = (*partPredicate)(iterData).ranges;
            }

            std::vector<std::pair<long long, V>> regVals = part->getValsWithInds(srcReg, true, partRanges);

            for (const auto& regVal : regVals) {
                if (resPredicate != nullptr && !(*resPredicate)(srcReg, regVal.second, iterData)) {
                    continue;
                }

                resCalls[resModObj][srcModObj][iterData.sectionName][part->id()][regVal.first] =
                    ResCall{regVal.second, iterData.query};
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResGroupCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResGroupCollect<K, V, KeyHash, KeyEqual>::remapGraphs(GraphGroups& graphGroups,
                                                                                                                          RemappedGraphs* remappedGraphs) {
        if (remappedGraphs == nullptr) {
            return graphGroups;
        }

        typename Remapper::RemapList remap;
        ByGraph<std::vector<std::string>> resTypes;

        for (const auto& srcEntry : remaps) {
            std::vector<typename Remapper::RemapTarget> targets;
            std::vector<std::string> currentResTypes;

            for (const auto& targetEntry : srcEntry.second) {
                currentResTypes.push_back(targetEntry.first);
                targets.push_back(targetEntry.second);
            }

            remap.emplace_back(srcEntry.first, std::move(targets));
            resTypes[srcEntry.first] = std::move(currentResTypes);
        }

        ByGraph<std::size_t> srcModObjOccurrences;
        Remapper graphGroupRemap(std::move(remap));

        graphGroupRemap.remapGraphs(graphGroups, [remappedGraphs, &resTypes, &srcModObjOccurrences](
                                                     GraphGroups& groups, Graph& fromGraph, const GraphId& fromId,
                                                     const GraphId&, const RenameFunc& renameFunc) -> Graph* {
            // newPartIds = false: the collected calls are keyed by part id, so the copy keeps the
            // ids the collection phase recorded. They are refreshed at the very end instead.
            Graph* result = groups.deepcopyGraph(fromGraph, true, false);

            std::size_t& occurrence = srcModObjOccurrences[fromId];
            const std::vector<std::string>& currentResTypes = resTypes[fromId];

            if (occurrence < currentResTypes.size()) {
                (*remappedGraphs)[fromId][currentResTypes[occurrence]] = RemappedGraph{result, renameFunc, true};
            }

            ++occurrence;
            return result;
        });

        return graphGroups;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::pair<bool, std::vector<typename ResGroupCollect<K, V, KeyHash, KeyEqual>::GraphId>> ResGroupCollect<K, V, KeyHash, KeyEqual>::isValidResGroupType(
        const std::string& resGroupType) const {
        if (groupedResBuilders.find(resGroupType) == groupedResBuilders.end()) {
            return {false, {}};
        }

        // The resource types common to both configuration maps, in resEdits' own insertion order --
        // matching DictTools.getCommonKeys's documented "first dict's order" contract.
        std::vector<GraphId> commonResTypes;
        for (const auto& entry : resEdits) {
            if (srcRegs.find(entry.first) != srcRegs.end()) {
                commonResTypes.push_back(entry.first);
            }
        }

        for (const GraphId& resType : commonResTypes) {
            auto it = resEdits.find(resType);
            if (it == resEdits.end() || it->second.find(resGroupType) == it->second.end()) {
                return {false, {}};
            }
        }

        return {true, commonResTypes};
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResGroupCollect<K, V, KeyHash, KeyEqual>::CollectedSections ResGroupCollect<K, V, KeyHash, KeyEqual>::getResCallNewNames(
        const GraphId& resModObj, const std::string& resGroupType,
        tsl::ordered_map<std::string, std::optional<Z3Predicate>>& resRootQueries,
        tsl::ordered_map<std::string, ResRootLocation>& resRootLocations, const std::string& modName) const {
        CollectedSections result;

        auto resCallsIt = resCalls.find(resModObj);
        if (resCallsIt == resCalls.end() || resCallsIt->second.empty()) {
            return result;
        }

        auto resEditsIt = resEdits.find(resModObj);
        if (resEditsIt == resEdits.end()) {
            return result;
        }

        auto resEditIt = resEditsIt->second.find(resGroupType);
        if (resEditIt == resEditsIt->second.end() || resEditIt->second == nullptr) {
            return result;
        }

        ResEdit& resEdit = *resEditIt->second;

        for (const auto& srcEntry : resCallsIt->second) {
            for (const auto& sectionEntry : srcEntry.second) {
                for (const auto& partEntry : sectionEntry.second) {
                    for (const auto& callEntry : partEntry.second) {
                        std::string resCall = resEdit.config.fileOf(callEntry.second.val);
                        std::optional<std::string> newResCall = resEdit.getFixResourceName(resCall, modName);

                        auto collected = resEdit.collectResourceName(resCall, newResCall.has_value() ? *newResCall : resCall);

                        result.insert_or_assign(collected.first, collected.second);
                        resRootQueries.insert_or_assign(collected.first, callEntry.second.query);
                        resRootLocations.insert_or_assign(collected.first,
                                                           ResRootLocation{resModObj, srcEntry.first, sectionEntry.first,
                                                                           partEntry.first, callEntry.first});
                    }
                }
            }
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResGroupCollect<K, V, KeyHash, KeyEqual>::Graph* ResGroupCollect<K, V, KeyHash, KeyEqual>::getResGraph(
        GraphGroups& graphGroups, const std::string& resGroupType, const GraphId& resModObj,
        ByGraph<CollectedSections>& resCallNewNames, tsl::ordered_map<std::string, std::optional<Z3Predicate>>& resRootQueries,
        tsl::ordered_map<std::string, ResRootLocation>& resRootLocations, Context* ctx, const std::string& modName) {
        auto resEditsIt = resEdits.find(resModObj);
        if (resEditsIt == resEdits.end()) {
            return nullptr;
        }

        auto resEditIt = resEditsIt->second.find(resGroupType);
        if (resEditIt == resEditsIt->second.end() || resEditIt->second == nullptr) {
            return nullptr;
        }

        if (ctx == nullptr) {
            return nullptr;
        }

        CollectedSections currentResCallNewNames = getResCallNewNames(resModObj, resGroupType, resRootQueries,
                                                                       resRootLocations, modName);
        resCallNewNames[resModObj] = currentResCallNewNames;

        // rename = false: the replicate phase renames every copy itself, folding the graph id in.
        // copySections = true: several resource-group types share one .ini file's sections.
        return resEditIt->second->getResGraph(currentResCallNewNames, *ctx, graphGroups, modName, false, true);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResGroupCollect<K, V, KeyHash, KeyEqual>::Graph* ResGroupCollect<K, V, KeyHash, KeyEqual>::collectAllResources(
        GraphGroups& graphGroups, const std::string& resGroupType, const GraphId& resModObj,
        ByGraph<CollectedSections>& resCallNewNames, GroupedResBuilder& builder, ResGroups& resGroups,
        std::unordered_set<GraphId, GraphIdHash>& collectedResTypes, Context* ctx, const std::string& modName) {
        (void)builder;

        tsl::ordered_map<std::string, std::optional<Z3Predicate>> resRootQueries;
        tsl::ordered_map<std::string, ResRootLocation> resRootLocations;

        Graph* graph = getResGraph(graphGroups, resGroupType, resModObj, resCallNewNames, resRootQueries, resRootLocations,
                                    ctx, modName);
        if (graph == nullptr) {
            return nullptr;
        }

        auto resEditIt = resEdits.find(resModObj);
        ResEdit& resEdit = *(resEditIt->second.find(resGroupType)->second);

        auto parts = graph->iterByQuery();
        while (parts.next()) {
            IterQueryData& iterData = parts.value();
            auto* part = iterData.part;

            std::vector<std::pair<long long, V>> fileVals = part->getValsWithInds(resEdit.config.filenameKey);
            if (fileVals.empty()) {
                continue;
            }

            auto locationIt = resRootLocations.find(iterData.rootSectionName);
            auto queryIt = resRootQueries.find(iterData.rootSectionName);
            if (locationIt == resRootLocations.end() || queryIt == resRootQueries.end() || !queryIt->second.has_value()) {
                continue;
            }

            Z3Predicate newQuery = combineQueries(*queryIt->second, iterData.query, graph->z3Ctx());

            for (const auto& fileVal : fileVals) {
                std::string val = resEdit.config.fileOf(fileVal.second);
                if (val == nullValue) {
                    continue;
                }

                ResGroup resGroup;
                resGroup.entries[resModObj] = ResGroupEntry{
                    resEdit.getFileId(resModObj, iterData.sectionName, part->id(), fileVal.first, val),
                    iterData.rootSectionName, locationIt->second, part->depth(), nullptr};
                resGroup.query = newQuery;
                resGroups.push_back(std::move(resGroup));
            }
        }

        collectedResTypes.insert(resModObj);
        return graph;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResGroupCollect<K, V, KeyHash, KeyEqual>::Graph* ResGroupCollect<K, V, KeyHash, KeyEqual>::collectSatisfyingResources(
        GraphGroups& graphGroups, const std::string& resGroupType, const GraphId& resModObj,
        ByGraph<CollectedSections>& resCallNewNames, GroupedResBuilder& builder, ResGroups& resGroups,
        std::unordered_set<GraphId, GraphIdHash>& collectedResTypes, Context* ctx, const std::string& modName) {
        (void)builder;

        tsl::ordered_map<std::string, std::optional<Z3Predicate>> resRootQueries;
        tsl::ordered_map<std::string, ResRootLocation> resRootLocations;

        Graph* graph = getResGraph(graphGroups, resGroupType, resModObj, resCallNewNames, resRootQueries, resRootLocations,
                                    ctx, modName);
        if (graph == nullptr) {
            return nullptr;
        }

        auto resEditIt = resEdits.find(resModObj);
        ResEdit& resEdit = *(resEditIt->second.find(resGroupType)->second);

        // Snapshotted before the loop: only the groups that existed *before* this resource type was
        // folded in are candidates to extend, so newly appended ones are not re-extended.
        std::size_t resGroupsLen = resGroups.size();

        auto parts = graph->iterByQuery();
        while (parts.next()) {
            IterQueryData& iterData = parts.value();
            auto* part = iterData.part;

            std::vector<std::pair<long long, V>> fileVals = part->getValsWithInds(resEdit.config.filenameKey);
            if (fileVals.empty()) {
                continue;
            }

            auto locationIt = resRootLocations.find(iterData.rootSectionName);
            auto queryIt = resRootQueries.find(iterData.rootSectionName);
            if (locationIt == resRootLocations.end() || queryIt == resRootQueries.end() || !queryIt->second.has_value()) {
                continue;
            }

            Z3Predicate newQuery = combineQueries(*queryIt->second, iterData.query, graph->z3Ctx());

            for (const auto& fileVal : fileVals) {
                std::string val = resEdit.config.fileOf(fileVal.second);
                if (val == nullValue) {
                    continue;
                }

                bool added = false;
                ResGroupEntry entry{resEdit.getFileId(resModObj, iterData.sectionName, part->id(), fileVal.first, val),
                                     iterData.rootSectionName, locationIt->second, part->depth(), nullptr};

                for (std::size_t i = 0; i < resGroupsLen; ++i) {
                    if (!resGroups[i].query.has_value()) {
                        continue;
                    }

                    // A real z3::solver decides '!=' natively -- unlike the sympy LRA check this
                    // replaced, no rewrite of '!=' into a disjunction of strict inequalities is
                    // needed first.
                    Z3Predicate newResGroupQuery = combineQueries(newQuery, *resGroups[i].query, graph->z3Ctx());
                    if (!newResGroupQuery.isSatisfiable()) {
                        continue;
                    }

                    ResGroup newResGroup;
                    newResGroup.entries = resGroups[i].entries;
                    newResGroup.entries[resModObj] = entry;
                    newResGroup.query = newResGroupQuery;
                    resGroups.push_back(std::move(newResGroup));

                    added = true;
                }

                if (!added) {
                    ResGroup resGroup;
                    resGroup.entries[resModObj] = entry;
                    resGroup.query = newQuery;

                    if (!resGroup.isMissing(collectedResTypes)) {
                        resGroups.push_back(std::move(resGroup));
                    }
                }
            }
        }

        collectedResTypes.insert(resModObj);

        // Anything that never picked up every collected resource type is not a real group.
        resGroups.erase(std::remove_if(resGroups.begin(), resGroups.end(), [&collectedResTypes](const ResGroup& resGroup) {
            return resGroup.isMissing(collectedResTypes);
        }), resGroups.end());

        return graph;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::collectResGroups(GraphGroups& graphGroups, const std::string& resGroupType,
                                                                     ByGraph<CollectedSections>& resCallNewNames,
                                                                     const std::vector<GraphId>& commonResTypes,
                                                                     ResGroups& resGroups, ByGraph<Graph*>& resGraphs,
                                                                     std::unordered_set<GraphId, GraphIdHash>& collectedResTypes,
                                                                     Context* ctx, const std::string& modName) {
        auto builderIt = groupedResBuilders.find(resGroupType);
        if (builderIt == groupedResBuilders.end() || builderIt->second == nullptr) {
            return;
        }

        bool firstCollected = false;

        for (const GraphId& resType : commonResTypes) {
            if (!firstCollected) {
                resGraphs[resType] = collectAllResources(graphGroups, resGroupType, resType, resCallNewNames,
                                                          *builderIt->second, resGroups, collectedResTypes, ctx, modName);
                firstCollected = true;
                continue;
            }

            resGraphs[resType] = collectSatisfyingResources(graphGroups, resGroupType, resType, resCallNewNames,
                                                             *builderIt->second, resGroups, collectedResTypes, ctx, modName);
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::collectResGraphNewNames(const std::string& resGroupType,
                                                                            const std::vector<GraphId>& commonResTypes,
                                                                            ByGraph<CollectedSections>& resCallNewNames,
                                                                            const std::string& modName) {
        for (const GraphId& resType : commonResTypes) {
            tsl::ordered_map<std::string, std::optional<Z3Predicate>> resRootQueries;
            tsl::ordered_map<std::string, ResRootLocation> resRootLocations;

            resCallNewNames[resType] = getResCallNewNames(resType, resGroupType, resRootQueries, resRootLocations, modName);
        }
    }


    // ---------------------------------------------------------------------------------------
    // replicate
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool ResGroupCollect<K, V, KeyHash, KeyEqual>::fileKeyExists(const std::string& fileKey,
                                                                  tsl::ordered_map<std::string, long long>& fileFreqs) {
        auto it = fileFreqs.find(fileKey);
        if (it == fileFreqs.end()) {
            return false;
        }

        if (it->second <= 1) {
            fileFreqs.erase(it);
        } else {
            fileFreqs[fileKey] = it->second - 1;
        }

        return true;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string ResGroupCollect<K, V, KeyHash, KeyEqual>::getResGraphId(std::size_t resGroupTypeId, std::size_t resTypeId,
                                                                         std::size_t graphCopyId) const {
        return std::to_string(id) + "_" + std::to_string(resGroupTypeId) + "_" + std::to_string(resTypeId) + "_" +
               std::to_string(graphCopyId);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::replicateResGraphs(
        const std::string& resGroupType, std::size_t resGroupTypeId,
        const ByGraph<tsl::ordered_map<std::string, long long>>& sectionFreqs,
        const ByGraph<tsl::ordered_map<std::string, long long>>& fileFreqs, const std::vector<GraphId>& commonResTypes,
        ByGraph<Graph*>& resGraphs, CollectedResources& collectedResources,
        ByGraph<std::vector<std::pair<Graph*, std::string>>>& combinedResGraphs, GraphGroups& graphGroups, Context* ctx,
        const std::string& modName) {
        // Copied, not aliased: each resource-group type consumes its own counts, and (with
        // resGroupTypesSameTopology) the caller's originals are reused for the next type.
        ByGraph<tsl::ordered_map<std::string, long long>> currentSectionFreqs = sectionFreqs;
        ByGraph<tsl::ordered_map<std::string, long long>> currentFileFreqs = fileFreqs;

        std::size_t resTypeId = 0;

        for (const GraphId& resType : commonResTypes) {
            auto resEditsIt = resEdits.find(resType);
            if (resEditsIt == resEdits.end()) {
                continue;
            }

            auto resEditIt = resEditsIt->second.find(resGroupType);
            if (resEditIt == resEditsIt->second.end() || resEditIt->second == nullptr) {
                continue;
            }

            auto graphIt = resGraphs.find(resType);
            if (graphIt == resGraphs.end() || graphIt->second == nullptr) {
                ++resTypeId;
                continue;
            }

            Graph* graph = graphIt->second;
            ResEdit& resEdit = *resEditIt->second;
            std::vector<std::pair<Graph*, std::string>>& currentCombinedGraphs = combinedResGraphs[resType];

            tsl::ordered_map<std::string, long long>& resFileFreqs = currentFileFreqs[resType];
            tsl::ordered_map<std::string, long long> resSectionFreqs = currentSectionFreqs[resType];
            std::size_t graphCopyId = 0;

            // One replica per round: every root section still owed a copy gets one, then every
            // owed count drops by one and the exhausted ones fall out.
            while (!resSectionFreqs.empty()) {
                std::string graphId = getResGraphId(resGroupTypeId, resTypeId, graphCopyId);
                Graph* newGraph = graphGroups.deepcopyGraph(*graph, true, false);

                if (resSectionFreqs.size() < newGraph->targetSectionNames().size()) {
                    std::vector<std::string> targets;
                    targets.reserve(resSectionFreqs.size());
                    for (const auto& entry : resSectionFreqs) {
                        targets.push_back(entry.first);
                    }

                    newGraph->build(std::nullopt, std::move(targets));
                }

                if (ctx != nullptr) {
                    // Captured rather than handed straight to the .ini file: a model only belongs to
                    // the .ini file once a group actually claims it (see connectResGroups).
                    ctx->beginCollectingResources();
                    resEdit.buildResModels(*newGraph, *ctx, modName,
                                            [&resFileFreqs](const std::string&, const std::string& fileKey) {
                                                return fileKeyExists(fileKey, resFileFreqs);
                                            },
                                            graphId, &resType);

                    for (auto& built : ctx->takeCollectedResources()) {
                        collectedResources[built.first].push_back(CollectedResource{built.second, graphId});
                    }

                    ctx->endCollectingResources();
                }

                currentCombinedGraphs.emplace_back(newGraph, graphId);
                ++graphCopyId;

                tsl::ordered_map<std::string, long long> remaining;
                for (const auto& entry : resSectionFreqs) {
                    long long freq = entry.second - 1;
                    if (freq > 0) {
                        remaining[entry.first] = freq;
                    }
                }

                resSectionFreqs = std::move(remaining);
            }

            ++resTypeId;
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::countAndReplicateResGraphs(
        const std::string& resGroupType, std::size_t resGroupTypeId, const ResGroups& resGroups,
        ByGraph<tsl::ordered_map<std::string, long long>>& sectionFreqs,
        ByGraph<tsl::ordered_map<std::string, long long>>& fileFreqs, const std::vector<GraphId>& commonResTypes,
        ByGraph<Graph*>& resGraphs, CollectedResources& collectedResources,
        ByGraph<std::vector<std::pair<Graph*, std::string>>>& combinedResGraphs, GraphGroups& graphGroups, Context* ctx,
        const std::string& modName) {
        for (const ResGroup& resGroup : resGroups) {
            for (const auto& entry : resGroup.entries) {
                sectionFreqs[entry.first][entry.second.rootSectionName] += 1;
                fileFreqs[entry.first][entry.second.fileKey] += 1;
            }
        }

        replicateResGraphs(resGroupType, resGroupTypeId, sectionFreqs, fileFreqs, commonResTypes, resGraphs,
                            collectedResources, combinedResGraphs, graphGroups, ctx, modName);
    }


    // ---------------------------------------------------------------------------------------
    // connect
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string ResGroupCollect<K, V, KeyHash, KeyEqual>::getNewSectionName(const std::string& sectionName,
                                                                             const CollectedSections& resNewCalls,
                                                                             const std::string& graphId) {
        auto it = resNewCalls.find(sectionName);
        return (it == resNewCalls.end() ? sectionName : it->second) + graphId;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::map<typename ResGroupCollect<K, V, KeyHash, KeyEqual>::ResRootLocation,
             typename ResGroupCollect<K, V, KeyHash, KeyEqual>::ResCallConnData> ResGroupCollect<K, V, KeyHash, KeyEqual>::connectResGroups(
        ResGroups& resGroups, CollectedResources& collectedResources, const ByGraph<CollectedSections>& resCallNewNames,
        const std::string& resGroupType) {
        // NOTE: a group that runs out of models part-way through has already consumed the models it
        // did claim -- they are not put back. That is the pure-Python original's own behaviour (it
        // breaks out of the same loop after popping), preserved rather than corrected.
        std::map<ResRootLocation, ResCallConnData> resCallConnData;

        auto builderIt = groupedResBuilders.find(resGroupType);
        GroupedResBuilder* builder = (builderIt == groupedResBuilders.end()) ? nullptr : builderIt->second;

        for (ResGroup& resGroup : resGroups) {
            bool isBuilt = true;
            std::vector<std::pair<GraphId, CollectedResource>> resGroupConnData;

            // Iterated by key, then mutated through .at(): tsl::ordered_map hands out const
            // values through its iterator even from a non-const begin().
            std::vector<GraphId> entryKeys;
            entryKeys.reserve(resGroup.entries.size());
            for (const auto& entry : resGroup.entries) {
                entryKeys.push_back(entry.first);
            }

            for (const GraphId& entryKey : entryKeys) {
                const std::string& fileKey = resGroup.entries.at(entryKey).fileKey;

                auto resourcesIt = collectedResources.find(fileKey);
                if (resourcesIt == collectedResources.end() || resourcesIt->second.empty()) {
                    isBuilt = false;
                    break;
                }

                CollectedResource currentResource = resourcesIt->second.front();
                collectedResources.at(fileKey).pop_front();

                resGroup.entries.at(entryKey).resource = currentResource.resource;
                resGroupConnData.emplace_back(entryKey, currentResource);
            }

            if (!isBuilt) {
                continue;
            }

            for (const auto& connEntry : resGroupConnData) {
                const ResGroupEntry& groupEntry = resGroup.entries.at(connEntry.first);

                auto newNamesIt = resCallNewNames.find(connEntry.first);
                static const CollectedSections noNewNames;
                const CollectedSections& newNames = (newNamesIt == resCallNewNames.end()) ? noNewNames : newNamesIt->second;

                std::string newSectionName = getNewSectionName(groupEntry.rootSectionName, newNames, connEntry.second.graphId);

                auto& connData = resCallConnData[groupEntry.rootLocation];
                if (connData.resCallers.empty()) {
                    connData.partDepth = groupEntry.partDepth;
                }

                connData.resCallers.emplace_back(newSectionName, resGroup.query);
            }

            // Only now is the grouped resource itself worth building -- see ResGroup's own note.
            if (builder != nullptr) {
                IniGroupedResource* groupedResource = builder->build();
                if (groupedResource != nullptr) {
                    for (const auto& entry : resGroup.entries) {
                        if (entry.second.resource != nullptr) {
                            builder->addResource(*groupedResource, entry.first, *entry.second.resource);
                        }
                    }

                    groupedResource->isBuilt = true;
                    builder->store(*groupedResource);
                }
            }
        }

        return resCallConnData;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::unique_ptr<IfTemplatePart>> ResGroupCollect<K, V, KeyHash, KeyEqual>::buildResIfCalls(
        const std::vector<std::pair<std::string, std::optional<Z3Predicate>>>& resCallers, const K& srcReg, int depth,
        Z3Context* targetZ3Ctx) const {
        // One entry per distinct section name, with every distinct call site's query OR'd together
        // -- "any one of them being taken suffices".
        //
        // NOTE: the pure-Python original wrote 'queries[sectionName] = sympy.Or(queries[sectionName])'
        // here, dropping the newly-seen query argument entirely. That was a real bug in this exact
        // line, fixed during the Z3 migration rather than preserved.
        tsl::ordered_map<std::string, std::optional<Z3Predicate>> queries;

        for (const auto& resCaller : resCallers) {
            auto it = queries.find(resCaller.first);
            if (it == queries.end()) {
                queries.insert_or_assign(resCaller.first, resCaller.second);
                continue;
            }

            if (!it->second.has_value()) {
                queries[resCaller.first] = resCaller.second;
                continue;
            }

            if (resCaller.second.has_value()) {
                queries[resCaller.first] = *it->second | *resCaller.second;
            }
        }

        std::vector<std::unique_ptr<IfTemplatePart>> result;
        std::optional<Z3Context> fallbackZ3Ctx;

        for (const auto& entry : queries) {
            if (!entry.second.has_value()) {
                continue;
            }

            Z3Predicate query = *entry.second;

            if (targetZ3Ctx != nullptr && !query.belongsTo(*targetZ3Ctx)) {
                std::optional<Z3Predicate> reparented = IfPredPart::reparent(query, *targetZ3Ctx);
                if (!reparented.has_value()) {
                    continue;
                }

                query = *reparented;
            }

            query = query.simplify();

            // Walks the Z3 expression directly -- no sympy-syntax text round trip, unlike the
            // deprecated pure-Python original's own getIfPredStr(ParseContext).
            std::optional<std::string> queryStr = IfPredPart::getIfPredStr(query);
            if (!queryStr.has_value()) {
                continue;
            }

            std::string ifPredPartSpace(static_cast<std::size_t>(std::max(depth, 0)), '\t');

            // 'ifCtx' is only actually read by IfPredPart's constructor when 'query' isn't already
            // supplied -- both parts built below always pass one (or are EndIf, which never has
            // one), so a throwaway fallback context is fine when there was no real target.
            Z3Context* ifCtx = targetZ3Ctx;
            if (ifCtx == nullptr) {
                if (!fallbackZ3Ctx.has_value()) {
                    fallbackZ3Ctx.emplace();
                }

                ifCtx = &(*fallbackZ3Ctx);
            }

            result.push_back(std::make_unique<IfPredPart>(ifPredPartSpace + IfPredPartTypeTools::getName(IfPredPartType::If) +
                                                              " " + *queryStr,
                                                          IfPredPartType::If, *ifCtx, nullptr, query));

            std::vector<std::pair<K, V>> kvps{{srcReg, valOfSectionName(entry.first)}};
            result.push_back(std::make_unique<ContentPart>(kvps, depth + 1));

            result.push_back(std::make_unique<IfPredPart>(IfPredPartTypeTools::getName(IfPredPartType::EndIf),
                                                          IfPredPartType::EndIf, *ifCtx));
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<std::unique_ptr<IfTemplatePart>> ResGroupCollect<K, V, KeyHash, KeyEqual>::splitIfContentPart(
        ContentPart& part, std::map<long long, std::vector<std::unique_ptr<IfTemplatePart>>>& ifPartsToAdd) {
        std::vector<long long> inds;
        inds.reserve(ifPartsToAdd.size());
        for (const auto& entry : ifPartsToAdd) {
            inds.push_back(entry.first);
        }

        // includeSplitKVP = false: the original reference is dropped, replaced by the if/endif
        // block built for it. includeEmptyParts = true so the split stays aligned with 'inds'.
        std::vector<std::unique_ptr<ContentPart>> splitParts = part.splitByInds(inds, false, true);

        std::vector<std::unique_ptr<IfTemplatePart>> result;
        std::size_t addInd = 0;

        for (std::size_t i = 0; i < splitParts.size(); ++i) {
            if (splitParts[i] != nullptr && !splitParts[i]->empty()) {
                result.push_back(std::move(splitParts[i]));
            }

            if (addInd >= ifPartsToAdd.size()) {
                continue;
            }

            auto it = ifPartsToAdd.begin();
            std::advance(it, static_cast<std::ptrdiff_t>(addInd));
            for (auto& newPart : it->second) {
                result.push_back(std::move(newPart));
            }

            ++addInd;
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResGroupCollect<K, V, KeyHash, KeyEqual>::Graph* ResGroupCollect<K, V, KeyHash, KeyEqual>::resolveToGraph(
        const std::string& resGroupType, const GraphId& srcModObj, GraphGroups& graphGroups, RemappedGraphs* remappedGraphs) {
        if (remappedGraphs == nullptr) {
            return Base::getGraph(graphGroups, srcModObj, false);
        }

        auto it = remappedGraphs->find(srcModObj);
        if (it == remappedGraphs->end()) {
            return nullptr;
        }

        auto subTypeIt = it->second.find(resGroupType);
        if (subTypeIt == it->second.end()) {
            return nullptr;
        }

        return subTypeIt->second.graph;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::connectResCalls(const std::string& resGroupType, GraphGroups& graphGroups,
                                                                    const std::map<ResRootLocation, ResCallConnData>& resCallConnData,
                                                                    RemappedGraphs* remappedGraphs) {
        // newCallParts[srcModObj][sectionName][partId][orderInd] -> the parts to splice in.
        //
        // NOTE: the pure-Python original additionally tracked "unconnected" call paths and filed an
        // empty part list for each. That set was always empty -- the flag it filtered on was only
        // ever written as True -- so it is simply not reproduced here.
        struct SectionParts {
            tsl::ordered_map<std::size_t, std::map<long long, std::vector<std::unique_ptr<IfTemplatePart>>>> byPartId;
        };

        ByGraph<tsl::ordered_map<std::string, SectionParts>> newCallParts;

        for (const auto& entry : resCallConnData) {
            const ResRootLocation& location = entry.first;

            auto srcRegsIt = srcRegs.find(location.resModObj);
            if (srcRegsIt == srcRegs.end()) {
                continue;
            }

            auto srcRegIt = srcRegsIt->second.find(location.srcModObj);
            if (srcRegIt == srcRegsIt->second.end()) {
                continue;
            }

            // The queries folded into the new IfPredParts have to live in whatever Z3Context the
            // destination graph actually uses -- resolved here, before anything is constructed.
            Graph* toGraph = resolveToGraph(resGroupType, location.srcModObj, graphGroups, remappedGraphs);
            Z3Context* targetZ3Ctx = (toGraph == nullptr) ? nullptr : toGraph->z3Ctx();

            newCallParts[location.srcModObj][location.sectionName].byPartId[location.partId][location.orderInd] =
                buildResIfCalls(entry.second.resCallers, srcRegIt->second, entry.second.partDepth, targetZ3Ctx);
        }

        std::vector<GraphId> srcKeys;
        srcKeys.reserve(newCallParts.size());
        for (const auto& srcEntry : newCallParts) {
            srcKeys.push_back(srcEntry.first);
        }

        for (const GraphId& srcKey : srcKeys) {
            Graph* toGraph = resolveToGraph(resGroupType, srcKey, graphGroups, remappedGraphs);
            if (toGraph == nullptr) {
                continue;
            }

            std::vector<std::string> sectionKeys;
            sectionKeys.reserve(newCallParts.at(srcKey).size());
            for (const auto& sectionEntry : newCallParts.at(srcKey)) {
                sectionKeys.push_back(sectionEntry.first);
            }

            for (const std::string& sectionKey : sectionKeys) {
                SectionParts& sectionParts = newCallParts.at(srcKey).at(sectionKey);

                Section* section = toGraph->getSection(sectionKey, false);
                if (section == nullptr) {
                    continue;
                }

                // Which part index each edited part currently sits at.
                std::map<std::size_t, std::vector<std::unique_ptr<IfTemplatePart>>> sectionNewParts;
                auto& parts = section->parts();

                for (std::size_t partInd = 0; partInd < parts.size(); ++partInd) {
                    if (parts[partInd] == nullptr) {
                        continue;
                    }

                    std::size_t partId = parts[partInd]->id();
                    if (sectionParts.byPartId.find(partId) == sectionParts.byPartId.end()) {
                        continue;
                    }

                    auto* contentPart = dynamic_cast<ContentPart*>(parts[partInd].get());
                    if (contentPart == nullptr) {
                        continue;
                    }

                    sectionNewParts[partInd] = splitIfContentPart(*contentPart, sectionParts.byPartId.at(partId));
                }

                if (sectionNewParts.empty()) {
                    continue;
                }

                // Each edited part is replaced in place by whatever it split into.
                std::vector<std::unique_ptr<IfTemplatePart>> rebuiltParts;
                for (std::size_t partInd = 0; partInd < parts.size(); ++partInd) {
                    auto it = sectionNewParts.find(partInd);
                    if (it == sectionNewParts.end()) {
                        if (parts[partInd] != nullptr) {
                            rebuiltParts.push_back(std::move(parts[partInd]));
                        }

                        continue;
                    }

                    for (auto& newPart : it->second) {
                        rebuiltParts.push_back(std::move(newPart));
                    }
                }

                parts = std::move(rebuiltParts);
                section->rebuild();
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::cleanResCallGraphs(RemappedGraphs* remappedGraphs) {
        if (remappedGraphs == nullptr) {
            return;
        }

        std::unordered_set<Graph*> visited;

        for (const auto& srcEntry : *remappedGraphs) {
            for (const auto& groupTypeEntry : srcEntry.second) {
                const RemappedGraph& remapped = groupTypeEntry.second;
                if (remapped.graph == nullptr || !visited.insert(remapped.graph).second) {
                    continue;
                }

                if (remapped.renameFunc) {
                    remapped.graph->rename(remapped.renameFunc);
                }

                if (remapped.partIdRefreshRequired) {
                    remapped.graph->refreshPartIds();
                }
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResGroupCollect<K, V, KeyHash, KeyEqual>::connectResGraphs(
        const std::string& resGroupType, ByGraph<std::vector<std::pair<Graph*, std::string>>>& combinedResGraphs,
        const ByGraph<CollectedSections>& resCallNewNames, GraphGroups& graphGroups) {
        std::vector<GraphId> resTypeKeys;
        resTypeKeys.reserve(combinedResGraphs.size());
        for (const auto& resTypeEntry : combinedResGraphs) {
            resTypeKeys.push_back(resTypeEntry.first);
        }

        for (const GraphId& resTypeKey : resTypeKeys) {
            auto resEditsIt = resEdits.find(resTypeKey);
            if (resEditsIt == resEdits.end()) {
                continue;
            }

            auto resEditIt = resEditsIt->second.find(resGroupType);
            if (resEditIt == resEditsIt->second.end() || resEditIt->second == nullptr) {
                continue;
            }

            std::vector<std::pair<Graph*, std::string>>& graphs = combinedResGraphs.at(resTypeKey);
            if (graphs.empty()) {
                continue;
            }

            auto newNamesIt = resCallNewNames.find(resTypeKey);
            static const CollectedSections noNewNames;
            const CollectedSections& newNames = (newNamesIt == resCallNewNames.end()) ? noNewNames : newNamesIt->second;

            for (auto& graphEntry : graphs) {
                const std::string& graphId = graphEntry.second;
                graphEntry.first->rename([&newNames, &graphId](const std::string& name) {
                    return getNewSectionName(name, newNames, graphId);
                });
                graphEntry.first->refreshPartIds();
            }

            std::vector<Graph*> rest;
            for (std::size_t i = 1; i < graphs.size(); ++i) {
                rest.push_back(graphs[i].first);
            }

            graphs[0].first->combine(rest);
            Base::addGraph(graphGroups, resEditIt->second->resModObj, graphs[0].first);
        }
    }


    // ---------------------------------------------------------------------------------------
    // driver
    // ---------------------------------------------------------------------------------------

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResGroupCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResGroupCollect<K, V, KeyHash, KeyEqual>::editImpl(
        GraphGroups& graphGroups, Context* ctx, const ModType* modType, const std::string& modName) {
        (void)modType;

        for (const auto& resTypeEntry : srcRegs) {
            for (const auto& srcEntry : resTypeEntry.second) {
                collectFromGraphGroup(graphGroups, resTypeEntry.first, srcEntry.first, srcEntry.second);
            }
        }

        RemappedGraphs remappedGraphsStorage;
        RemappedGraphs* remappedGraphs = remaps.empty() ? nullptr : &remappedGraphsStorage;
        remapGraphs(graphGroups, remappedGraphs);

        ResGroups resGroups;
        ResGroups resGroupsCopy;
        ByGraph<Graph*> resGraphs;
        ByGraph<CollectedSections> resCallNewNames;
        bool resGroupsCollected = false;
        std::unordered_set<GraphId, GraphIdHash> collectedResTypes;
        ByGraph<tsl::ordered_map<std::string, long long>> sectionFreqs;
        ByGraph<tsl::ordered_map<std::string, long long>> fileFreqs;
        CollectedResources collectedResources;
        ByGraph<std::vector<std::pair<Graph*, std::string>>> combinedResGraphs;
        std::size_t resGroupTypeId = 0;

        for (const std::string& resGroupType : resGroupTypes_) {
            auto validity = isValidResGroupType(resGroupType);
            if (!validity.first) {
                continue;
            }

            const std::vector<GraphId>& commonResTypes = validity.second;

            resCallNewNames.clear();
            collectedResources.clear();
            combinedResGraphs.clear();

            if (!resGroupTypesSameTopology) {
                resGraphs.clear();
                resGroups.clear();
                collectedResTypes.clear();
                sectionFreqs.clear();
                fileFreqs.clear();
            }

            if (!resGroupsCollected) {
                collectResGroups(graphGroups, resGroupType, resCallNewNames, commonResTypes, resGroups, resGraphs,
                                  collectedResTypes, ctx, modName);
                countAndReplicateResGraphs(resGroupType, resGroupTypeId, resGroups, sectionFreqs, fileFreqs, commonResTypes,
                                            resGraphs, collectedResources, combinedResGraphs, graphGroups, ctx, modName);

                if (resGroupTypesSameTopology) {
                    resGroupsCopy = resGroups;
                }
            } else {
                resGroups = resGroupsCopy;
                collectResGraphNewNames(resGroupType, commonResTypes, resCallNewNames, modName);
                replicateResGraphs(resGroupType, resGroupTypeId, sectionFreqs, fileFreqs, commonResTypes, resGraphs,
                                    collectedResources, combinedResGraphs, graphGroups, ctx, modName);
            }

            auto resCallConnData = connectResGroups(resGroups, collectedResources, resCallNewNames, resGroupType);
            connectResCalls(resGroupType, graphGroups, resCallConnData, remappedGraphs);
            connectResGraphs(resGroupType, combinedResGraphs, resCallNewNames, graphGroups);

            if (resGroupTypesSameTopology && !resGroupsCollected) {
                resGroupsCollected = true;
            }

            ++resGroupTypeId;
        }

        cleanResCallGraphs(remappedGraphs);
        return graphGroups;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResGroupCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResGroupCollect<K, V, KeyHash, KeyEqual>::editWithContext(
        GraphGroups& graphGroups, Context& ctx, const ModType* modType, const std::string& modName) {
        clear();
        editImpl(graphGroups, &ctx, modType, modName);
        clear();
        return graphGroups;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResGroupCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResGroupCollect<K, V, KeyHash, KeyEqual>::edit(
        GraphGroups& graphGroups, const ModType* modType, const std::string& modName) {
        clear();
        return editImpl(graphGroups, nullptr, modType, modName);
    }
}

#endif
