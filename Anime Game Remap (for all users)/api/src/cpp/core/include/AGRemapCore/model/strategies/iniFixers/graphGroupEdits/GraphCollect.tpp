#ifndef AGRemapCore_GraphCollect_TPP
#define AGRemapCore_GraphCollect_TPP

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

#include <type_traits>
#include <utility>

#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphCollect<K, V, KeyHash, KeyEqual>::GraphCollect(ByGraph<std::vector<K>> keys, ByGraph<std::vector<K>> contextKeys, std::string name):
        keys(std::move(keys)), contextKeys(std::move(contextKeys)), name(std::move(name)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::vector<typename GraphCollect<K, V, KeyHash, KeyEqual>::Row>& GraphCollect<K, V, KeyHash, KeyEqual>::rows() const {
        return rows_;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GraphCollect<K, V, KeyHash, KeyEqual>::clear() {
        rows_.clear();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GraphCollect<K, V, KeyHash, KeyEqual>::fileInto(IniParseData& data) const {
        // IniParseData holds std::string rows
        if constexpr (std::is_same_v<K, std::string> && std::is_same_v<V, std::string>) {
            if (!name.empty()) {
                data.setCollected(name, rows_);
            }
        } else {
            (void)data;
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GraphCollect<K, V, KeyHash, KeyEqual>::addRows(const GraphId& graphId, const K& key, const IterData& iterData,
                                                         const std::string& rootSection, const std::optional<Z3Predicate>& condition) {
        auto* part = iterData.part;
        if (part == nullptr) {
            return;
        }

        auto partPredicate = partPredicates.find(graphId);
        auto valPredicate = valPredicates.find(graphId);
        auto context = contextKeys.find(graphId);

        // No predicate means no restriction at all -- std::nullopt, not an empty range list (which would
        // restrict everything away).
        std::optional<typename Graph::ContentPart::RangeSpec> partRanges;
        if (partPredicate != partPredicates.end() && partPredicate->second) {
            partRanges = partPredicate->second(iterData).ranges;
        }

        for (const auto& [line, value] : part->getValsWithInds(key, true, partRanges)) {
            if (valPredicate != valPredicates.end() && valPredicate->second && !valPredicate->second(key, value, iterData)) {
                continue;
            }

            Row row;
            row.graphId = graphId;
            row.section = iterData.sectionName;
            row.rootSection = rootSection;
            row.partId = part->id();
            row.line = line;
            row.key = key;
            row.value = value;
            row.condition = condition;
            if (context != contextKeys.end()) {
                for (const K& contextKey : context->second) {
                    row.context.emplace_back(contextKey, iterData.colouring != nullptr ? iterData.colouring->getIndVals(contextKey)
                                                                                       : KeyState());
                }
            }

            rows_.push_back(std::move(row));
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GraphCollect<K, V, KeyHash, KeyEqual>::collectKey(GraphGroups& graphGroups, const GraphId& graphId, const K& key) {
        Graph* graph = Base::getGraph(graphGroups, graphId, false);
        if (graph == nullptr) {
            return;
        }

        std::optional<KeySet> colourKeys;
        if (auto tracked = keysToTrack.find(graphId); tracked != keysToTrack.end()) {
            colourKeys = tracked->second;
        }

        bool colour = trackKeysGlobal;
        if (!trackKeysIsGlobal) {
            auto tracked = trackKeys.find(graphId);
            colour = tracked != trackKeys.end() && tracked->second;
        }

        // the context keys are always tracked: they are what a row records
        auto context = contextKeys.find(graphId);
        if (context != contextKeys.end() && !context->second.empty()) {
            if (colourKeys.has_value()) {
                colourKeys->insert(context->second.begin(), context->second.end());
            } else if (!colour) {
                colourKeys = KeySet(context->second.begin(), context->second.end());
            }
            colour = true;
        }

        if (withConditions) {
            auto parts = graph->iterByQuery({}, false, 1, colour, colourKeys);
            while (parts.next()) {
                auto& queryData = parts.value();
                const IterData iterData(queryData.sectionName, queryData.section, queryData.part, queryData.state, queryData.colouring);
                addRows(graphId, key, iterData, queryData.rootSectionName, queryData.query);
            }
            return;
        }

        auto parts = graph->iterByContentPart(1, colour, colourKeys);
        while (parts.next()) {
            addRows(graphId, key, parts.value(), "", std::nullopt);
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GraphCollect<K, V, KeyHash, KeyEqual>::collectAll(GraphGroups& graphGroups) {
        for (const auto& [graphId, named] : keys) {
            for (const K& key : named) {
                collectKey(graphGroups, graphId, key);
            }
        }

        for (const auto& [graphId, match] : keyMatches) {
            // the keys named outright are searched already
            KeySet searched;
            if (auto named = keys.find(graphId); named != keys.end()) {
                searched.insert(named->second.begin(), named->second.end());
            }

            for (const K& key : matchedKeys(graphGroups, graphId, match, std::move(searched))) {
                collectKey(graphGroups, graphId, key);
            }
        }
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::vector<K> GraphCollect<K, V, KeyHash, KeyEqual>::matchedKeys(GraphGroups& graphGroups, const GraphId& graphId,
                                                                      const KeyMatch& match, KeySet searched) const {
        std::vector<K> matched;
        Graph* graph = match ? Base::getGraph(graphGroups, graphId, false) : nullptr;
        if (graph == nullptr) {
            return matched;
        }

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
                        matched.push_back(item.key);
                    }
                }
            }
        }

        return matched;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphCollect<K, V, KeyHash, KeyEqual>::GraphSettings GraphCollect<K, V, KeyHash, KeyEqual>::takeGraphSettings() {
        GraphSettings settings;
        settings.keys = std::move(keys);
        settings.keyMatches = std::move(keyMatches);
        settings.contextKeys = std::move(contextKeys);
        settings.partPredicates = std::move(partPredicates);
        settings.valPredicates = std::move(valPredicates);
        settings.trackKeys = std::move(trackKeys);
        settings.keysToTrack = std::move(keysToTrack);
        return settings;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void GraphCollect<K, V, KeyHash, KeyEqual>::putGraphSettings(GraphSettings settings) {
        keys = std::move(settings.keys);
        keyMatches = std::move(settings.keyMatches);
        contextKeys = std::move(settings.contextKeys);
        partPredicates = std::move(settings.partPredicates);
        valPredicates = std::move(settings.valPredicates);
        trackKeys = std::move(settings.trackKeys);
        keysToTrack = std::move(settings.keysToTrack);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphCollect<K, V, KeyHash, KeyEqual>::GraphSettings GraphCollect<K, V, KeyHash, KeyEqual>::inEveryGroup(
            const GraphSettings& configured, const GraphGroups& graphGroups) const {
        // group by group, each group's entries in their configured order
        const auto perGroup = [&graphGroups](const auto& map) {
            std::remove_cv_t<std::remove_reference_t<decltype(map)>> result;
            for (std::size_t g = 0; g < graphGroups.size(); ++g) {
                for (const auto& [id, value] : map) {
                    GraphId inGroup(g, id.modObj.first, id.modObj.second);
                    if (Base::getGraph(graphGroups, inGroup, false) != nullptr) {
                        result.emplace(std::move(inGroup), value);
                    }
                }
            }
            return result;
        };

        GraphSettings settings;
        settings.keys = perGroup(configured.keys);
        settings.keyMatches = perGroup(configured.keyMatches);
        settings.contextKeys = perGroup(configured.contextKeys);
        settings.partPredicates = perGroup(configured.partPredicates);
        settings.valPredicates = perGroup(configured.valPredicates);
        settings.trackKeys = perGroup(configured.trackKeys);
        settings.keysToTrack = perGroup(configured.keysToTrack);
        return settings;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphCollect<K, V, KeyHash, KeyEqual>::GraphGroups& GraphCollect<K, V, KeyHash, KeyEqual>::edit(GraphGroups& graphGroups,
                                                                                                             const ModType* modType,
                                                                                                             const std::string& modName) {
        (void)modType;
        (void)modName;
        clear();
        if (!everyGroup) {
            collectAll(graphGroups);
            return graphGroups;
        }

        // run on the per-group copies, and put the configured settings back however the run ends
        GraphSettings configured = takeGraphSettings();
        putGraphSettings(inEveryGroup(configured, graphGroups));
        try {
            collectAll(graphGroups);
        } catch (...) {
            putGraphSettings(std::move(configured));
            throw;
        }

        putGraphSettings(std::move(configured));
        return graphGroups;
    }
}

#endif
