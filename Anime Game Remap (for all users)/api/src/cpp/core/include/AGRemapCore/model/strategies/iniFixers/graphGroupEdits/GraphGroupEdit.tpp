#ifndef AGRemapCore_GraphGroupEdit_TPP
#define AGRemapCore_GraphGroupEdit_TPP

#include <algorithm>
#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphGroupEdit(std::vector<IniEdits> edits, bool trackKeysIsGlobal, bool trackKeysGlobal):
        edits(std::move(edits)), trackKeysIsGlobal(trackKeysIsGlobal), trackKeysGlobal(trackKeysGlobal) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphGroupEdit<K, V, KeyHash, KeyEqual>::Graph* GraphGroupEdit<K, V, KeyHash, KeyEqual>::filterGroupEdit(Graph& graph, PartEditKind filterKind,
                                                                                                                      const std::vector<PartEdit*>& filterGroup,
                                                                                                                      IniFile* ini, const ModType* modType,
                                                                                                                      const std::string& modName,
                                                                                                                      const std::vector<PartFilter>& keyFilters,
                                                                                                                      bool trackKeys,
                                                                                                                      const std::optional<KeySet>& keysToTrack) {
        Graph* result = &graph;
        std::size_t filterGroupLen = filterGroup.size();
        std::size_t keyFiltersLen = keyFilters.size();

        // NOTE ON keyFilters INDEXING: 'i' below is the index *within this same-kind run*, not
        // within the caller's whole filter list -- so for a second or later run, the key filters
        // restart from index 0 rather than staying aligned with their own edits. That is exactly
        // what the pure-Python original did (editSectionGraph hands the full 'keyFilters' to every
        // run, and each run indexes it from 0), and it is preserved deliberately rather than
        // "corrected": it is invisible whenever every edit for a graph is the same kind (the only
        // shape any real call site or test uses, since a run then spans the whole list), and
        // changing it would silently repoint existing callers' filters at different edits. Flagged
        // here rather than fixed.

        if (filterKind == PartEditKind::GraphEdit) {
            for (std::size_t i = 0; i < filterGroupLen; ++i) {
                // An empty PartFilter is the equivalent of the pure-Python original's
                // 'defaultFilter', which returned Ranges.createFull() -- ie. no restriction.
                //
                // BUGFIX vs. the pure-Python original: this branch read a 'keyFiltersLen' that was
                // only ever assigned in the *other* branch, so it raised NameError for every graph
                // edit that reached here; and its no-'ini' variant built a 2-parameter lambda that
                // was then called with 3 arguments, raising TypeError. Both are fixed here (there
                // is one code path, and it takes the filter as a real argument), which is why this
                // whole branch is reachable at all now.
                // Passed as a pointer into 'keyFilters' rather than a copy -- see
                // PartEdit::editGraph's own note on why the address is load-bearing.
                const PartFilter* keyFilter = (i < keyFiltersLen) ? &keyFilters[i] : nullptr;

                // The group's own key-tracking settings are handed down here: unlike a register
                // edit (below), a graph edit walks the graph itself, so nothing this class builds
                // would otherwise reach it. See PartEdit::editGraph's own note.
                Graph* edited = filterGroup[i]->editGraph(*result, ini, modType, modName, keyFilter, trackKeys, keysToTrack);
                if (edited != nullptr) {
                    result = edited;
                }
            }

            return result;
        }

        if (filterKind != PartEditKind::RegEdit) {
            return result;
        }

        auto parts = result->iterByContentPart(1, trackKeys, keysToTrack);
        while (parts.next()) {
            IterData& iterData = parts.value();

            for (std::size_t i = 0; i < filterGroupLen; ++i) {
                OrderRanges keyRanges = (i >= keyFiltersLen || !keyFilters[i]) ? OrderRanges::createFull()
                                                                              : keyFilters[i](iterData, modType, ini);
                if (keyRanges.isEmpty()) {
                    continue;
                }

                filterGroup[i]->editPart(*iterData.part, iterData.sectionName, ini, modType, modName, keyRanges);

                if (iterData.colouring != nullptr) {
                    iterData.colouring->updateColouring(*iterData.part, keysToTrack, false);
                }
            }

            // BUGFIX vs. the pure-Python original: it ended this loop with "if (partChanged):
            // parts[i] = part", where 'i' was the *filter* index left over from the inner loop, not
            // the part's own index in the section -- so a section whose part count differed from
            // its filter count had an unrelated part silently overwritten. No write-back happens
            // here instead, which is both correct and the only thing the C++ types allow: every
            // edit mutates 'part' in place and BaseRegEdit::edit's own contract is to return the
            // same part, so there is never a different part to write back (see PartEdit::editPart,
            // which returns void for exactly this reason).
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphGroupEdit<K, V, KeyHash, KeyEqual>::Graph* GraphGroupEdit<K, V, KeyHash, KeyEqual>::editSectionGraph(Graph& graph, const std::vector<PartEdit*>& filters,
                                                                                                                       IniFile* ini, const ModType* modType,
                                                                                                                       const std::string& modName,
                                                                                                                       const std::vector<PartFilter>& keyFilters,
                                                                                                                       bool trackKeys,
                                                                                                                       const std::optional<KeySet>& keysToTrack) {
        Graph* result = &graph;
        std::vector<PartEdit*> filterGroup;
        PartEditKind filterKind = PartEditKind::None;

        if (!filters.empty()) {
            filterKind = filters[0]->kind();
        }

        for (PartEdit* filter : filters) {
            PartEditKind currentFilterKind = filter->kind();

            if (filterKind == currentFilterKind) {
                filterGroup.push_back(filter);
                continue;
            }

            result = filterGroupEdit(*result, filterKind, filterGroup, ini, modType, modName, keyFilters, trackKeys, keysToTrack);

            filterKind = currentFilterKind;
            filterGroup.clear();
            filterGroup.push_back(filter);
        }

        if (!filterGroup.empty()) {
            result = filterGroupEdit(*result, filterKind, filterGroup, ini, modType, modName, keyFilters, trackKeys, keysToTrack);
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphGroups& GraphGroupEdit<K, V, KeyHash, KeyEqual>::editImpl(GraphGroups& graphGroups, IniFile* ini,
                                                                                                                     const ModType* modType, const std::string& modName) {
        std::size_t minIniFileLen = std::min(graphGroups.size(), edits.size());

        for (std::size_t i = 0; i < minIniFileLen; ++i) {
            const IniEdits& iniEdits = edits[i];

            // Iterates the *graphs*, skipping any without edits -- matching the pure-Python
            // original's "for modObj in iniGraphs: if (modObj not in iniEdits): continue", rather
            // than iterating the edits. The two differ in order whenever both are non-empty.
            for (const ModObj& modObj : graphGroups.modObjs(i)) {
                auto editsIt = iniEdits.edits.find(modObj);
                if (editsIt == iniEdits.edits.end()) {
                    continue;
                }

                Graph* iniGraph = graphGroups.getGraph(i, modObj);
                if (iniGraph == nullptr) {
                    continue;
                }

                static const std::vector<PartFilter> noKeyFilters;
                auto keyFiltersIt = iniEdits.keyFilters.find(modObj);
                const std::vector<PartFilter>& objKeyFilters = (keyFiltersIt == iniEdits.keyFilters.end()) ? noKeyFilters
                                                                                                           : keyFiltersIt->second;

                static const std::optional<KeySet> noKeysToTrack;
                auto keysToTrackIt = iniEdits.keysToTrack.find(modObj);
                const std::optional<KeySet>& objKeysToTrack = (keysToTrackIt == iniEdits.keysToTrack.end()) ? noKeysToTrack
                                                                                                            : keysToTrackIt->second;

                bool objTrackKeys = trackKeysGlobal;
                if (!trackKeysIsGlobal) {
                    auto trackKeysIt = iniEdits.trackKeys.find(modObj);
                    objTrackKeys = (trackKeysIt == iniEdits.trackKeys.end()) ? false : trackKeysIt->second;
                }

                Graph* edited = editSectionGraph(*iniGraph, editsIt->second, ini, modType, modName, objKeyFilters,
                                                  objTrackKeys, objKeysToTrack);

                if (edited != nullptr && edited != iniGraph) {
                    graphGroups.addGraph(i, modObj, edited);
                }
            }
        }

        return graphGroups;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphGroups& GraphGroupEdit<K, V, KeyHash, KeyEqual>::editFromIni(GraphGroups& graphGroups, IniFile* ini,
                                                                                                                        const ModType* modType, const std::string& modName) {
        return editImpl(graphGroups, ini, modType, modName);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphGroupEdit<K, V, KeyHash, KeyEqual>::GraphGroups& GraphGroupEdit<K, V, KeyHash, KeyEqual>::edit(GraphGroups& graphGroups, const ModType* modType,
                                                                                                                 const std::string& modName) {
        return editImpl(graphGroups, nullptr, modType, modName);
    }
}

#endif
