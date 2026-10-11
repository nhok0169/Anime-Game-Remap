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

#ifndef AGRemapCore_LynaeSkeleton_H
#define AGRemapCore_LynaeSkeleton_H

#include <array>
#include <map>
#include <unordered_map>
#include <vector>


namespace AGRemapCore {
    /**
     * @brief
     @rst
     The merged ids Lynae's 3.7 update renumbered, as ``{3.6 id: 3.7 id}`` -- every id not listed is
     the same in both. See :cpp:member:`WWMIFixerConfig::skeletonNumberings`
     @endrst
     */
    const std::unordered_map<long long, long long>& lynaeRenumbered36To37();

    /**
     * @brief
     @rst
     Her rest-pose bone centroids in the 3.7 numbering, off ``Data/Mod Downloads/WuWa/Lynae/3_7`` --
     see :cpp:member:`WWMIFixerConfig::referenceBoneCentroids`
     @endrst
     */
    const std::unordered_map<long long, std::array<double, 3>>& lynaeBoneCentroids();

    /**
     * @brief
     @rst
     Each component's local bone -> her 3.6 merged id, for a mod from before WWMI's merged skeleton
     (every such mod of hers is a 3.6 export) -- see :cpp:member:`WWMIFixerConfig::sourceVgMaps`
     @endrst
     */
    const std::map<int, std::vector<int>>& lynae36VgMaps();

    /**
     * @brief
     @rst
     For each of LynaePeppermint's shape-key slots, which of Lynae's keys it is (``-1``: none) -- see
     :cpp:member:`WWMIFixerConfig::shapeKeyOrder`
     @endrst
     */
    const std::vector<long long>& lynaeToPeppermintShapeKeys();
}

#endif
