#ifndef AGRemapCore_RegFillMissing_TPP
#define AGRemapCore_RegFillMissing_TPP

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/model/files/IniFile.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    RegFillMissing<K, V, KeyHash, KeyEqual>::RegFillMissing(K reg, FillMissingFunc fillMissing,
                                                             RegFillMissingMode fillMode, bool dependOnDownload,
                                                             bool trackKeys, std::optional<KeySet> keysToTrack):
        reg(std::move(reg)), fillMissing(std::move(fillMissing)), fillMode(fillMode), dependOnDownload(dependOnDownload),
        trackKeys(trackKeys), keysToTrack(std::move(keysToTrack)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegFillMissing<K, V, KeyHash, KeyEqual>::FillMissingFunc RegFillMissing<K, V, KeyHash, KeyEqual>::makeFillMissing(
            K reg, V value, bool toFront) {
        return [reg = std::move(reg), value = std::move(value), toFront](ContentPart& part) {
            if (toFront) {
                part.addKVPToFront(reg, value);
            } else {
                part.addKVP(reg, value);
            }
        };
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegFillMissing<K, V, KeyHash, KeyEqual>::FillMissingFunc RegFillMissing<K, V, KeyHash, KeyEqual>::makeFillMissing(
            std::vector<std::pair<K, V>> kvps, bool toFront) {
        return [kvps = std::move(kvps), toFront](ContentPart& part) {
            if (toFront) {
                part.addKVPsToFront(kvps);
            } else {
                part.addKVPs(kvps);
            }
        };
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    bool RegFillMissing<K, V, KeyHash, KeyEqual>::effectiveTrackKeys(bool callerTrackKeys) const {
        return trackKeys || callerTrackKeys;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    const std::optional<typename RegFillMissing<K, V, KeyHash, KeyEqual>::KeySet>&
    RegFillMissing<K, V, KeyHash, KeyEqual>::effectiveKeysToTrack(const std::optional<KeySet>& callerKeysToTrack) const {
        return keysToTrack.has_value() ? keysToTrack : callerKeysToTrack;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegFillMissing<K, V, KeyHash, KeyEqual>::Graph& RegFillMissing<K, V, KeyHash, KeyEqual>::fillMissingGraph(
            Graph& graph, const K& reg, const FillMissingFunc& fillMissing, const PartSelection& selection) {
        if (!fillMissing) {
            return graph;
        }

        std::unordered_map<std::string, std::set<ContentPart*>> parts = graph.getKeyMissingParts(reg);

        // The same part can be reachable from more than one section, and getKeyMissingParts reports
        // it under each -- fill it exactly once, matching the pure-Python original's own
        // 'partVisited' set.
        std::unordered_set<ContentPart*> missing;
        for (const auto& entry : parts) {
            for (ContentPart* part : entry.second) {
                if (part != nullptr) {
                    missing.insert(part);
                }
            }
        }

        if (missing.empty()) {
            return graph;
        }

        // Nothing to gate on -- fill straight from the missing set, with no graph walk at all. This
        // is the pre-selection behaviour, preserved exactly (including its unordered fill order).
        if (!selection.partFilter && !selection.trackKeys) {
            for (ContentPart* part : missing) {
                fillMissing(*part);
            }

            return graph;
        }

        // getKeyMissingParts and iterByContentPart both walk outwards from roots_, so the parts
        // reachable here are exactly the ones 'missing' can hold -- no missing part is silently
        // skipped just because the walk never reaches it.
        std::unordered_set<ContentPart*> filled;
        auto walk = graph.iterByContentPart(1, selection.trackKeys, selection.keysToTrack);

        while (walk.next()) {
            IterData& iterData = walk.value();
            ContentPart* part = iterData.part;

            if (part == nullptr || missing.count(part) == 0 || filled.count(part) != 0) {
                continue;
            }

            if (selection.partFilter) {
                OrderRanges accepted = selection.partFilter(iterData, selection.modType, selection.ini);

                // An empty Ranges means "skip this part" -- the same convention GraphGroupEdit
                // already applies to its own register edits. A non-empty result's actual ranges are
                // not consulted; see PartSelection::partFilter's own note.
                if (accepted.isEmpty()) {
                    continue;
                }
            }

            fillMissing(*part);
            filled.insert(part);

            // Reflect the fill in the running colouring, so a later part's filter sees the KVP this
            // one just gained -- mirrors GraphGroupEdit's own post-edit updateColouring call.
            if (iterData.colouring != nullptr) {
                iterData.colouring->updateColouring(*part, selection.keysToTrack, false);
            }
        }

        return graph;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegFillMissing<K, V, KeyHash, KeyEqual>::Graph& RegFillMissing<K, V, KeyHash, KeyEqual>::addCover(
            Graph& graph, const K& reg, const FillMissingFunc& fillMissing, const PartSelection& selection) {
        if (!fillMissing) {
            return graph;
        }

        std::unordered_map<std::string, bool> covered = graph.rootsAreFullyCovered(reg);

        bool needCover = false;
        for (const auto& entry : covered) {
            if (!entry.second) {
                needCover = true;
                break;
            }
        }

        if (!needCover) {
            return graph;
        }

        // Nothing to gate on -- cover every root, the pre-selection behaviour preserved exactly.
        if (!selection.partFilter) {
            for (Section* section : graph.getRootSections()) {
                if (section == nullptr) {
                    continue;
                }

                ContentPart* topPart = section->addTopContentPart();
                if (topPart != nullptr) {
                    fillMissing(*topPart);
                }
            }

            return graph;
        }

        for (const std::string& rootName : graph.roots()) {
            Section* section = graph.getSection(rootName);
            if (section == nullptr) {
                continue;
            }

            // The root's own first IfContentPart -- the one addTopContentPart would reuse, or
            // insert before. It is what the filter gets to discriminate on; see addCover's own note
            // on why the colouring here is necessarily empty.
            ContentPart* firstPart = nullptr;
            for (const auto& part : section->parts()) {
                firstPart = dynamic_cast<ContentPart*>(part.get());
                if (firstPart != nullptr) {
                    break;
                }
            }

            // A section with no IfContentPart at all has nothing to discriminate on, so it is
            // accepted rather than silently dropped.
            if (firstPart != nullptr) {
                Colouring colouring;
                IterData iterData(rootName, section, firstPart, 1, selection.trackKeys ? &colouring : nullptr);

                OrderRanges accepted = selection.partFilter(iterData, selection.modType, selection.ini);
                if (accepted.isEmpty()) {
                    continue;
                }
            }

            ContentPart* topPart = section->addTopContentPart();
            if (topPart != nullptr) {
                fillMissing(*topPart);
            }
        }

        return graph;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegFillMissing<K, V, KeyHash, KeyEqual>::Graph& RegFillMissing<K, V, KeyHash, KeyEqual>::editFromIni(
            Graph& graph, IniFile* ini, const ModType* modType, const std::string& modName, const PartFilter& partFilter,
            bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        if (!dependOnDownload) {
            return edit(graph, modType, modName, partFilter, trackKeys, keysToTrack);
        }

        // See this method's doc comment -- no .ini file to read a mode off is treated as Normal,
        // the mode that adds no download-specific behaviour of its own.
        DownloadMode downloadMode = (ini != nullptr) ? ini->downloadMode : DownloadMode::Normal;

        if (downloadMode == DownloadMode::Disabled) {
            return graph;
        }

        if (downloadMode == DownloadMode::Always) {
            graph.normalize();
        }

        return editImpl(graph, ini, modType, modName, partFilter, trackKeys, keysToTrack);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegFillMissing<K, V, KeyHash, KeyEqual>::Graph& RegFillMissing<K, V, KeyHash, KeyEqual>::edit(
            Graph& graph, const ModType* modType, const std::string& modName, const PartFilter& partFilter,
            bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        // No .ini file to hand a filter -- see editImpl's own note on why that argument exists.
        return editImpl(graph, nullptr, modType, modName, partFilter, trackKeys, keysToTrack);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename RegFillMissing<K, V, KeyHash, KeyEqual>::Graph& RegFillMissing<K, V, KeyHash, KeyEqual>::editImpl(
            Graph& graph, IniFile* ini, const ModType* modType, const std::string& modName, const PartFilter& partFilter,
            bool callerTrackKeys, const std::optional<KeySet>& callerKeysToTrack) {
        (void)modName;

        PartSelection selection;
        selection.partFilter = partFilter;
        selection.modType = modType;
        selection.ini = ini;

        // This edit's own settings are combined with the caller's rather than replacing them -- see
        // effectiveTrackKeys/effectiveKeysToTrack.
        selection.trackKeys = effectiveTrackKeys(callerTrackKeys);
        selection.keysToTrack = effectiveKeysToTrack(callerKeysToTrack);

        if (fillMode == RegFillMissingMode::TopdownCover) {
            addCover(graph, reg, fillMissing, selection);
        } else if (fillMode == RegFillMissingMode::FillMissing) {
            fillMissingGraph(graph, reg, fillMissing, selection);
        }

        return graph;
    }
}

#endif
