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

#ifndef AGRemapCore_ChangliSkeleton_H
#define AGRemapCore_ChangliSkeleton_H

#include <array>
#include <map>
#include <unordered_map>
#include <vector>


namespace AGRemapCore {
    /**
     * @brief
     @rst
     The merged ids of Changli's 1.1 skeleton that her later updates renumbered, as
     ``{1.1 id: 1.4 id}`` -- every id not listed is the same in both. See
     :cpp:member:`WWMIFixerConfig::skeletonNumberings`
     @endrst
     */
    const std::unordered_map<long long, long long>& changliRenumbered11To14();

    /**
     * @brief
     @rst
     The merged ids of her 1.2 / 1.3 skeleton that her 1.4 update renumbered, as ``{1.2 id: 1.4 id}``
     @endrst
     */
    const std::unordered_map<long long, long long>& changliRenumbered12To14();

    /**
     * @brief
     @rst
     Her rest-pose bone centroids in the 1.4 numbering, off ``Data/Mod Downloads/WuWa/Changli/3_7`` --
     see :cpp:member:`WWMIFixerConfig::referenceBoneCentroids`
     @endrst
     */
    const std::unordered_map<long long, std::array<double, 3>>& changliBoneCentroids();

    /**
     * @brief
     @rst
     Each component's local bone -> her current merged id, for a mod from before WWMI's merged skeleton
     -- see :cpp:member:`WWMIFixerConfig::sourceVgMaps`
     @endrst
     */
    const std::map<int, std::vector<int>>& changliVgMaps();

    /**
     * @brief
     @rst
     For each of ChangliNymph's shape-key slots, which of Changli's keys it is (``-1``: none) -- see
     :cpp:member:`WWMIFixerConfig::shapeKeyOrder`
     @endrst
     */
    const std::vector<long long>& changliToNymphShapeKeys();
}

#endif
