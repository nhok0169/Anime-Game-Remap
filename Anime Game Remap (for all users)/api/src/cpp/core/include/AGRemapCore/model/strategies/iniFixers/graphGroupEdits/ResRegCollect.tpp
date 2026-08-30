#ifndef AGRemapCore_ResRegCollect_TPP
#define AGRemapCore_ResRegCollect_TPP

#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    ResRegCollect<K, V, KeyHash, KeyEqual>::ResRegCollect(ByGraph<K> srcRegs, tsl::ordered_map<std::string, ResEdit*> resEdits):
        srcRegs(std::move(srcRegs)), resEdits(std::move(resEdits)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResRegCollect<K, V, KeyHash, KeyEqual>::clear() {
        resCalls.clear();

        for (const auto& entry : resEdits) {
            if (entry.second != nullptr) {
                entry.second->clear();
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResRegCollect<K, V, KeyHash, KeyEqual>::collectFromGraphGroup(GraphGroups& graphGroups, const GraphId& srcModObj,
                                                                        const K& srcReg) {
        Graph* graph = Base::getGraph(graphGroups, srcModObj, false);
        if (graph == nullptr) {
            return;
        }

        auto partPredicateIt = partPredicates.find(srcModObj);
        auto resPredicateIt = resPredicates.find(srcModObj);
        auto keysToTrackIt = keysToTrack.find(srcModObj);

        std::optional<std::unordered_set<K, KeyHash, KeyEqual>> currentKeysToTrack;
        if (keysToTrackIt != keysToTrack.end()) {
            currentKeysToTrack = keysToTrackIt->second;
        }

        bool currentTrackKeys = trackKeysGlobal;
        if (!trackKeysIsGlobal) {
            auto trackKeysIt = trackKeys.find(srcModObj);
            currentTrackKeys = (trackKeysIt == trackKeys.end()) ? false : trackKeysIt->second;
        }

        auto parts = graph->iterByContentPart(1, currentTrackKeys, currentKeysToTrack);
        while (parts.next()) {
            IterData& iterData = parts.value();
            auto* part = iterData.part;

            // No predicate means no restriction at all -- std::nullopt, not an empty range list
            // (which would restrict everything away).
            std::optional<typename Graph::ContentPart::RangeSpec> partRanges;
            if (partPredicateIt != partPredicates.end() && partPredicateIt->second) {
                partRanges = partPredicateIt->second(iterData).ranges;
            }

            std::vector<std::pair<long long, V>> regVals = part->getValsWithInds(srcReg, true, partRanges);

            for (const auto& regVal : regVals) {
                if (resPredicateIt != resPredicates.end() && resPredicateIt->second &&
                    !resPredicateIt->second(srcReg, regVal.second, iterData)) {
                    continue;
                }

                resCalls[srcModObj][iterData.sectionName][part->id()].push_back(ResCall{regVal.first, regVal.second});
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResRegCollect<K, V, KeyHash, KeyEqual>::remapGraphs(GraphGroups& graphGroups,
                                                                                                                      RemappedGraphs* remappedGraphs) {
        if (remappedGraphs == nullptr) {
            return graphGroups;
        }

        // Flattened into the (source graph -> list of targets) shape GraphGroupRemap takes, keeping
        // a parallel list of which resource subtype each target came from so the remapped graph can
        // be filed back under it.
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

        graphGroupRemap.remapGraphs(graphGroups, [this, remappedGraphs, &resTypes, &srcModObjOccurrences](
                                                     GraphGroups& groups, Graph& fromGraph, const GraphId& fromId,
                                                     const GraphId&, const RenameFunc& renameFunc) -> Graph* {
            // newPartIds = false: the collected calls are keyed by part id, so the copy has to keep
            // the ids the collection phase already recorded. They are refreshed at the very end
            // instead (see partIdRefreshRequired).
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
    void ResRegCollect<K, V, KeyHash, KeyEqual>::collectResourceNames(const std::string& resSubType, ResEdit& resEdit,
                                                                       typename ResEdit::CollectedSections& collectedSections,
                                                                       GraphGroups& graphGroups, RemappedGraphs* remappedGraphs,
                                                                       bool editGraph, const std::string& modName) {
        bool hasRemappedGraphs = remappedGraphs != nullptr;

        for (const auto& srcEntry : resCalls) {
            const GraphId& fromModObj = srcEntry.first;

            Graph* toGraph = nullptr;
            RenameFunc renameFunc;
            bool partIdRefreshRequired = false;

            if (hasRemappedGraphs) {
                auto remappedIt = remappedGraphs->find(fromModObj);
                if (remappedIt != remappedGraphs->end()) {
                    auto subTypeIt = remappedIt->second.find(resSubType);
                    if (subTypeIt != remappedIt->second.end()) {
                        toGraph = subTypeIt->second.graph;
                        renameFunc = subTypeIt->second.renameFunc;
                        partIdRefreshRequired = subTypeIt->second.partIdRefreshRequired;
                    }
                }
            } else {
                toGraph = Base::getGraph(graphGroups, fromModObj, false);
            }

            for (const auto& sectionEntry : srcEntry.second) {
                for (const auto& partEntry : sectionEntry.second) {
                    for (const ResCall& resCall : partEntry.second) {
                        std::string val = resEdit.config.fileOf(resCall.val);
                        std::optional<std::string> newVal = resEdit.getFixResourceName(val, modName);

                        if (newVal.has_value()) {
                            if (editGraph && toGraph != nullptr) {
                                auto* section = toGraph->getSection(sectionEntry.first, false);
                                if (section != nullptr) {
                                    // KNOWN, PRE-EXISTING GAP (not introduced by this port): when
                                    // 'toGraph' is a *remapped* copy, this lookup misses every time.
                                    // The calls were collected against the original graph's part
                                    // ids, and IniSectionGraph::deepcopy assigns fresh ids to the
                                    // copy's parts regardless of its 'newPartIds' flag (that flag
                                    // only skips the extra refreshPartIds pass; the clone itself
                                    // still draws new ids) -- so the reference simply is not
                                    // rewritten in the remapped copy. Reproduced empirically
                                    // against the live build.
                                    //
                                    // The pure-Python original indexed 'partsById' directly here,
                                    // so the same miss raised KeyError rather than skipping. This
                                    // keeps the collection half working (the resource's own graph
                                    // and models are still built correctly) instead of taking the
                                    // whole edit down, and is flagged rather than "fixed": actually
                                    // fixing it means correlating parts positionally instead of by
                                    // id, which is a behaviour change this port was not asked to
                                    // make.
                                    auto partIt = section->partsById().find(partEntry.first);
                                    if (partIt != section->partsById().end()) {
                                        auto* part = dynamic_cast<typename Graph::ContentPart*>(partIt->second);
                                        if (part != nullptr) {
                                            part->setValByInd(resCall.orderInd, resEdit.config.valOfFile(*newVal));
                                        }
                                    }
                                }
                            }

                            auto collected = resEdit.collectResourceName(val, *newVal);
                            collectedSections.insert_or_assign(collected.first, collected.second);
                        } else {
                            auto collected = resEdit.collectResourceName(val, val);
                            collectedSections.insert_or_assign(collected.first, collected.second);
                        }
                    }
                }
            }

            if (toGraph != nullptr && renameFunc) {
                toGraph->rename(renameFunc);
            }

            if (toGraph != nullptr && partIdRefreshRequired) {
                toGraph->refreshPartIds();
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResRegCollect<K, V, KeyHash, KeyEqual>::editImpl(GraphGroups& graphGroups,
                                                                                                                   Context* ctx,
                                                                                                                   const ModType* modType,
                                                                                                                   const std::string& modName) {
        (void)modType;
        clear();

        bool hasIni = ctx != nullptr && ctx->hasIni();

        // copySections only matters once more than one subtype is built from the same collected
        // set -- otherwise every subtype would be editing the same sections in place.
        bool copySections = resEdits.size() > 1;

        for (const auto& srcEntry : srcRegs) {
            collectFromGraphGroup(graphGroups, srcEntry.first, srcEntry.second);
        }

        RemappedGraphs remappedGraphsStorage;
        RemappedGraphs* remappedGraphs = remaps.empty() ? nullptr : &remappedGraphsStorage;
        remapGraphs(graphGroups, remappedGraphs);

        for (const auto& resEditEntry : resEdits) {
            if (resEditEntry.second == nullptr) {
                continue;
            }

            typename ResEdit::CollectedSections collectedSections;
            collectResourceNames(resEditEntry.first, *resEditEntry.second, collectedSections, graphGroups, remappedGraphs,
                                  hasIni, modName);

            if (hasIni) {
                resEditEntry.second->buildResources(collectedSections, *ctx, graphGroups, modName, {}, copySections);
            }
        }

        return graphGroups;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResRegCollect<K, V, KeyHash, KeyEqual>::editWithContext(GraphGroups& graphGroups,
                                                                                                                          Context& ctx,
                                                                                                                          const ModType* modType,
                                                                                                                          const std::string& modName) {
        editImpl(graphGroups, &ctx, modType, modName);
        clear();
        return graphGroups;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResRegCollect<K, V, KeyHash, KeyEqual>::edit(GraphGroups& graphGroups,
                                                                                                               const ModType* modType,
                                                                                                               const std::string& modName) {
        return editImpl(graphGroups, nullptr, modType, modName);
    }
}

#endif
