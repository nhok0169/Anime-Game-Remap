#ifndef AGRemapCore_GraphGroupRemap_TPP
#define AGRemapCore_GraphGroupRemap_TPP

#include <cstddef>
#include <utility>

#include "AGRemapCore/model/IniNamingTools.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphGroupRemap<K, V, KeyHash, KeyEqual>::GraphGroupRemap(RemapList remap): remap(std::move(remap)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphGroupRemap<K, V, KeyHash, KeyEqual>::Graph* GraphGroupRemap<K, V, KeyHash, KeyEqual>::copyGraph(GraphGroups& graphGroups, Graph& fromGraph,
                                                                                                                  const GraphId& modObj, const GraphId& newModObj,
                                                                                                                  const RenameFunc& renameFunc, const std::string& modName) {
        Graph* result = graphGroups.deepcopyGraph(fromGraph);
        if (result == nullptr) {
            return nullptr;
        }

        if (renameFunc) {
            result->rename(renameFunc);
        } else {
            const auto& fromObj = modObj.modObj;
            const auto& toObj = newModObj.modObj;
            result->rename([&modName, &fromObj, &toObj](const std::string& oldSectionName) {
                return IniNamingTools::getObjRemapFixName(oldSectionName, modName, fromObj, toObj);
            });
        }

        return result;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphGroupRemap<K, V, KeyHash, KeyEqual>::GraphGroups& GraphGroupRemap<K, V, KeyHash, KeyEqual>::remapGraphs(GraphGroups& graphGroups,
                                                                                                                          const CreateToGraph& createToGraph) {
        // The pure-Python original models this by temporarily turning the flat list of groups into
        // a list-of-lists (one bucket per .ini file, plus one trailing bucket for "a brand-new .ini
        // file"), filling the buckets, then flattening back. Doing that literally would mean
        // rebuilding the whole container, which this class cannot do -- 'graphGroups' is a live
        // view over a caller-owned sequence (see IIniGraphGroups). The bucket structure is instead
        // tracked as a pair of index vectors over the *flat* list, and groups are inserted directly
        // at the position the flatten would have put them. Same result, no rebuild.
        std::size_t graphGroupLen = graphGroups.size();

        std::vector<std::size_t> bucketStart(graphGroupLen + 1);
        std::vector<std::size_t> bucketLen(graphGroupLen + 1, 1);

        for (std::size_t i = 0; i < graphGroupLen; ++i) {
            bucketStart[i] = i;
        }

        bucketStart[graphGroupLen] = graphGroupLen;
        bucketLen[graphGroupLen] = 0;

        auto appendToBucket = [&](std::size_t bucket) {
            graphGroups.insertGroup(bucketStart[bucket] + bucketLen[bucket]);
            ++bucketLen[bucket];

            for (std::size_t j = bucket + 1; j <= graphGroupLen; ++j) {
                ++bucketStart[j];
            }
        };

        auto popBucketFront = [&](std::size_t bucket) {
            graphGroups.removeGroup(bucketStart[bucket]);
            --bucketLen[bucket];

            for (std::size_t j = bucket + 1; j <= graphGroupLen; ++j) {
                --bucketStart[j];
            }
        };

        // ----- retrieve the graphs to remove -----
        // Parallel to 'remap', holding the removed graph for each entry whose source graph existed
        // (nullptr otherwise) -- the pure-Python original's 'removedGraphs' dict, kept in the same
        // insertion order its own iteration depended on.
        std::vector<Graph*> removedGraphs(remap.size(), nullptr);

        for (std::size_t i = 0; i < remap.size(); ++i) {
            const GraphId& srcModObj = remap[i].first;
            if (srcModObj.iniIndex > graphGroupLen || bucketLen[srcModObj.iniIndex] == 0) {
                // The bucket-is-empty half of this guard has no equivalent in the pure-Python
                // original, which would raise IndexError reaching for its first group. It can only
                // be hit by naming the trailing "brand-new .ini file" bucket as a *source*, which
                // by construction never holds a graph to remap from -- so skipping is the only
                // sensible reading, and crashing was never the intent.
                continue;
            }

            removedGraphs[i] = graphGroups.removeGraph(bucketStart[srcModObj.iniIndex], srcModObj.modObj);
        }

        // ----- remap the graphs -----
        for (std::size_t i = 0; i < remap.size(); ++i) {
            Graph* fromGraph = removedGraphs[i];
            if (fromGraph == nullptr) {
                continue;
            }

            const GraphId& srcModObj = remap[i].first;

            for (const RemapTarget& target : remap[i].second) {
                std::size_t bucket = target.id.iniIndex;
                if (bucket > graphGroupLen) {
                    bucket = graphGroupLen;
                }

                if (bucketLen[bucket] == 0) {
                    appendToBucket(bucket);
                }

                // Walk forward until a group in this .ini file's bucket has no graph under the
                // target key yet, creating one on demand -- see this method's own note on
                // collisions.
                std::size_t groupInd = 0;
                while (graphGroups.getGraph(bucketStart[bucket] + groupInd, target.id.modObj) != nullptr) {
                    if (groupInd >= bucketLen[bucket] - 1) {
                        appendToBucket(bucket);
                    }

                    ++groupInd;
                }

                Graph* toGraph = createToGraph(graphGroups, *fromGraph, srcModObj, target.id, target.renameFunc);
                if (toGraph == nullptr) {
                    continue;
                }

                graphGroups.addGraph(bucketStart[bucket] + groupInd, target.id.modObj, toGraph);
            }
        }

        // ----- remove any now-empty original groups -----
        for (std::size_t i = 0; i < remap.size(); ++i) {
            if (removedGraphs[i] == nullptr) {
                continue;
            }

            std::size_t bucket = remap[i].first.iniIndex;
            // Same "the pure-Python original would IndexError here" guard as above -- an earlier
            // entry sharing this .ini file may already have dropped its first group.
            if (bucket > graphGroupLen || bucketLen[bucket] == 0) {
                continue;
            }

            if (graphGroups.graphCount(bucketStart[bucket]) == 0) {
                popBucketFront(bucket);
            }
        }

        return graphGroups;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphGroupRemap<K, V, KeyHash, KeyEqual>::GraphGroups& GraphGroupRemap<K, V, KeyHash, KeyEqual>::edit(GraphGroups& graphGroups, const ModType* modType,
                                                                                                                   const std::string& modName) {
        (void)modType;

        return remapGraphs(graphGroups, [&modName](GraphGroups& groups, Graph& fromGraph, const GraphId& fromId,
                                                    const GraphId& toId, const RenameFunc& renameFunc) {
            return copyGraph(groups, fromGraph, fromId, toId, renameFunc, modName);
        });
    }
}

#endif
