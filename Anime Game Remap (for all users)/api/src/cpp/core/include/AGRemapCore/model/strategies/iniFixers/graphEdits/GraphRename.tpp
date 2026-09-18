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
