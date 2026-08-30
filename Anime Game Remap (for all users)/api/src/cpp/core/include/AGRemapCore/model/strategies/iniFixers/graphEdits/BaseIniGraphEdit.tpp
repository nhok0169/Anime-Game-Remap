#ifndef AGRemapCore_BaseIniGraphEdit_TPP
#define AGRemapCore_BaseIniGraphEdit_TPP


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseIniGraphEdit<K, V, KeyHash, KeyEqual>::Graph& BaseIniGraphEdit<K, V, KeyHash, KeyEqual>::editFromIni(
            Graph& graph, IniFile* ini, const ModType* modType, const std::string& modName, const PartFilter& partFilter,
            bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        // 'ini' is deliberately unused -- see this method's doc comment. The pure-Python original
        // drops it here too; only overriding subclasses actually read it.
        (void)ini;
        return edit(graph, modType, modName, partFilter, trackKeys, keysToTrack);
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename BaseIniGraphEdit<K, V, KeyHash, KeyEqual>::Graph& BaseIniGraphEdit<K, V, KeyHash, KeyEqual>::edit(
            Graph& graph, const ModType* modType, const std::string& modName, const PartFilter& partFilter,
            bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        (void)modType;
        (void)modName;
        (void)partFilter;
        (void)trackKeys;
        (void)keysToTrack;
        return graph;
    }
}

#endif
