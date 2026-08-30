#ifndef AGRemapCore_GraphRename_TPP
#define AGRemapCore_GraphRename_TPP

#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphRename<K, V, KeyHash, KeyEqual>::GraphRename(RenameFunc renameFunc): renameFunc(std::move(renameFunc)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphRename<K, V, KeyHash, KeyEqual>::Graph& GraphRename<K, V, KeyHash, KeyEqual>::edit(
            Graph& graph, const ModType* modType, const std::string& modName, const PartFilter& partFilter,
            bool trackKeys, const std::optional<KeySet>& keysToTrack) {
        (void)modType;
        (void)modName;
        (void)partFilter;
        (void)trackKeys;
        (void)keysToTrack;

        // An empty std::function is this class's stand-in for "no rename function at all" -- the
        // pure-Python original could not express that (its constructor argument was mandatory), so
        // there is nothing to match here beyond not crashing.
        if (!renameFunc) {
            return graph;
        }

        graph.rename(renameFunc);
        return graph;
    }
}

#endif
