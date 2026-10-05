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

#ifndef AGRemapCore_GraphGroupRemove_TPP
#define AGRemapCore_GraphGroupRemove_TPP

#include <algorithm>
#include <utility>


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    GraphGroupRemove<K, V, KeyHash, KeyEqual>::GraphGroupRemove(std::optional<std::vector<std::size_t>> iniIndices):
        iniIndices(std::move(iniIndices)) {}

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename GraphGroupRemove<K, V, KeyHash, KeyEqual>::GraphGroups& GraphGroupRemove<K, V, KeyHash, KeyEqual>::edit(
            GraphGroups& graphGroups, const ModType* modType, const std::string& modName) {
        (void)modType;
        (void)modName;

        if (!iniIndices.has_value()) {
            while (graphGroups.size() > 0) {
                graphGroups.removeGroup(graphGroups.size() - 1);
            }

            return graphGroups;
        }

        // Highest first, once each: removing a group shifts every later index down by one.
        std::vector<std::size_t> toRemove = *iniIndices;
        std::sort(toRemove.begin(), toRemove.end(), std::greater<std::size_t>());
        toRemove.erase(std::unique(toRemove.begin(), toRemove.end()), toRemove.end());

        for (std::size_t iniIndex : toRemove) {
            if (iniIndex < graphGroups.size()) {
                graphGroups.removeGroup(iniIndex);
            }
        }

        return graphGroups;
    }
}

#endif
