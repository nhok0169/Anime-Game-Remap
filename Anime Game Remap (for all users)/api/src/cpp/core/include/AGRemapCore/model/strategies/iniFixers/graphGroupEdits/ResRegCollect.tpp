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

#ifndef AGRemapCore_ResRegCollect_TPP
#define AGRemapCore_ResRegCollect_TPP

#include <type_traits>
#include <utility>

#include "AGRemapCore/model/IniNamingTools.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::ResPredicate ResRegCollect<K, V, KeyHash, KeyEqual>::skipsKeywords(std::vector<std::string> keywords) {
        return [keywords = std::move(keywords)](const K&, const V& value, const IterData&) {
            for (const std::string& keyword : keywords) {
                if (IniNamingTools::hasKeywordPrefix(value, keyword)) {
                    return false;
                }
            }
            return true;
        };
    }

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
                                            // ADDED rather than overwritten when a target key is
                                            // named: the edit is then a COPY -- the register it was
                                            // read from keeps pointing at the original file. See
                                            // bindToReg.
                                            if (bindToReg.has_value()) {
                                                part->addKVP(*bindToReg, resEdit.config.valOfFile(*newVal));
                                            } else {
                                                part->setValByInd(resCall.orderInd, resEdit.config.valOfFile(*newVal));
                                            }
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
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphSettings ResRegCollect<K, V, KeyHash, KeyEqual>::takeGraphSettings() {
        GraphSettings settings;
        settings.srcRegs = std::move(srcRegs);
        settings.extraSrcRegs = std::move(extraSrcRegs);
        settings.srcRegMatches = std::move(srcRegMatches);
        settings.partPredicates = std::move(partPredicates);
        settings.resPredicates = std::move(resPredicates);
        settings.remaps = std::move(remaps);
        settings.trackKeys = std::move(trackKeys);
        settings.keysToTrack = std::move(keysToTrack);
        return settings;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void ResRegCollect<K, V, KeyHash, KeyEqual>::putGraphSettings(GraphSettings settings) {
        srcRegs = std::move(settings.srcRegs);
        extraSrcRegs = std::move(settings.extraSrcRegs);
        srcRegMatches = std::move(settings.srcRegMatches);
        partPredicates = std::move(settings.partPredicates);
        resPredicates = std::move(settings.resPredicates);
        remaps = std::move(settings.remaps);
        trackKeys = std::move(settings.trackKeys);
        keysToTrack = std::move(settings.keysToTrack);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphSettings ResRegCollect<K, V, KeyHash, KeyEqual>::inEveryGroup(
            const GraphSettings& configured, const GraphGroups& graphGroups) const {
        // group by group, each group's entries in their configured order
        const auto perGroup = [&graphGroups](const auto& map, auto adjust) {
            std::remove_cv_t<std::remove_reference_t<decltype(map)>> result;
            for (std::size_t g = 0; g < graphGroups.size(); ++g) {
                for (const auto& [id, value] : map) {
                    GraphId inGroup(g, id.modObj.first, id.modObj.second);
                    if (Base::getGraph(graphGroups, inGroup, false) != nullptr) {
                        auto copy = value;
                        adjust(copy, g, id);
                        result.emplace(std::move(inGroup), std::move(copy));
                    }
                }
            }
            return result;
        };
        const auto asIs = [](auto&, std::size_t, const GraphId&) {};

        GraphSettings settings;
        settings.srcRegs = perGroup(configured.srcRegs, asIs);
        settings.extraSrcRegs = perGroup(configured.extraSrcRegs, asIs);
        settings.srcRegMatches = perGroup(configured.srcRegMatches, asIs);
        settings.partPredicates = perGroup(configured.partPredicates, asIs);
        settings.resPredicates = perGroup(configured.resPredicates, asIs);
        settings.trackKeys = perGroup(configured.trackKeys, asIs);
        settings.keysToTrack = perGroup(configured.keysToTrack, asIs);

        // a target keeps its distance from its source's group
        settings.remaps = perGroup(configured.remaps, [](auto& targets, std::size_t g, const GraphId& configuredSrc) {
            for (auto it = targets.begin(); it != targets.end();) {
                const long long index = static_cast<long long>(g) + static_cast<long long>(it->second.id.iniIndex)
                                        - static_cast<long long>(configuredSrc.iniIndex);
                if (index < 0) {
                    it = targets.erase(it);
                    continue;
                }

                it.value().id.iniIndex = static_cast<std::size_t>(index);
                ++it;
            }
        });
        return settings;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResRegCollect<K, V, KeyHash, KeyEqual>::editImpl(GraphGroups& graphGroups,
                                                                                                                   Context* ctx,
                                                                                                                   const ModType* modType,
                                                                                                                   const std::string& modName) {
        (void)modType;
        if (!everyGroup) {
            return collectAndBuild(graphGroups, ctx, modName);
        }

        // run on the per-group copies, and put the configured settings back however the run ends
        GraphSettings configured = takeGraphSettings();
        putGraphSettings(inEveryGroup(configured, graphGroups));
        try {
            collectAndBuild(graphGroups, ctx, modName);
        } catch (...) {
            putGraphSettings(std::move(configured));
            throw;
        }

        putGraphSettings(std::move(configured));
        return graphGroups;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResRegCollect<K, V, KeyHash, KeyEqual>::collectAndBuild(GraphGroups& graphGroups,
                                                                                                                          Context* ctx,
                                                                                                                          const std::string& modName) {
        clear();

        bool hasIni = ctx != nullptr && ctx->hasIni();

        // ALWAYS COPY. createGraph is handed ctx.sectionIfTemplates() -- the .ini FILE'S OWN parsed
        // sections -- so without this the resource graph holds pointers straight into them, and
        // ResReplace::buildResModels' `part->setValByInd(ind, newVal)` rewrites the mod's real
        // `filename = ` line rather than the copy the fix renders.
        //
        // This used to be `resEdits.size() > 1`, which named the hazard too narrowly: "two subtypes
        // built from one collected set would edit the same sections in place". The same thing
        // happens between two FIXERS of the same .ini file, and there it is not cosmetic. Jean is
        // the first character to remap onto two targets, and the JeanCN fixer left
        // `[ResourceJeanBlend]` saying `filename = JeanJeanCNRemapBlend.buf`; the JeanSea fixer then
        // took that as its source and produced JeanJeanCNRemapJeanSeaRemapBlend.buf -- a Blend.buf
        // remapped TWICE. In game JeanSea came out warped, vertices stretched into spikes
        // (Images/Jean/JeanSeaWarped.jpg), which is what wrong vertex-group weights look like.
        //
        // Copying costs one section clone per collected resource and makes the edit idempotent,
        // which is what it should always have been: a fix must never mutate the parse of the file
        // it is reading.
        const bool copySections = true;

        for (const auto& srcEntry : srcRegs) {
            collectFromGraphGroup(graphGroups, srcEntry.first, srcEntry.second);
        }
        for (const auto& srcEntry : extraSrcRegs) {
            for (const K& srcReg : srcEntry.second) {
                collectFromGraphGroup(graphGroups, srcEntry.first, srcReg);
            }
        }
        for (const auto& [graphId, match] : srcRegMatches) {
            Graph* graph = match ? Base::getGraph(graphGroups, graphId, false) : nullptr;
            if (graph == nullptr) {
                continue;
            }

            // the registers named outright are searched already
            std::unordered_set<K, KeyHash, KeyEqual> searched;
            if (auto named = srcRegs.find(graphId); named != srcRegs.end()) {
                searched.insert(named->second);
            }
            if (auto named = extraSrcRegs.find(graphId); named != extraSrcRegs.end()) {
                searched.insert(named->second.begin(), named->second.end());
            }

            std::vector<K> regs;
            for (const auto& section : graph->sections()) {
                if (section.second == nullptr) {
                    continue;
                }

                for (const auto& part : section.second->parts()) {
                    const auto* content = dynamic_cast<const typename Graph::ContentPart*>(part.get());
                    if (content == nullptr) {
                        continue;
                    }

                    for (const auto& item : content->items()) {
                        if (match(item.key) && searched.insert(item.key).second) {
                            regs.push_back(item.key);
                        }
                    }
                }
            }

            for (const K& reg : regs) {
                collectFromGraphGroup(graphGroups, graphId, reg);
            }
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
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResRegCollect<K, V, KeyHash, KeyEqual>::editFromIni(
            GraphGroups& graphGroups, IniFile* ini, const ModType* modType, const std::string& modName) {
        if constexpr (std::is_same_v<K, std::string> && std::is_same_v<V, std::string>) {
            if (ini != nullptr) {
                // Owned for exactly this call: the context is scratch state for one edit pass, and
                // nothing outlives it -- the resources it collects are handed to the .ini file.
                IniFileResEditContext resCtx(ini);
                return editImpl(graphGroups, &resCtx, modType, modName);
            }
        }

        return edit(graphGroups, modType, modName);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename ResRegCollect<K, V, KeyHash, KeyEqual>::GraphGroups& ResRegCollect<K, V, KeyHash, KeyEqual>::edit(GraphGroups& graphGroups,
                                                                                                               const ModType* modType,
                                                                                                               const std::string& modName) {
        return editImpl(graphGroups, nullptr, modType, modName);
    }
}

#endif
