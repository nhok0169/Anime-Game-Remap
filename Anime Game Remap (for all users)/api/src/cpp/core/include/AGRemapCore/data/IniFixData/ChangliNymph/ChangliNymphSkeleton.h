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

#ifndef AGRemapCore_ChangliNymphSkeleton_H
#define AGRemapCore_ChangliNymphSkeleton_H

#include <map>
#include <vector>


namespace AGRemapCore {
    /**
     * @brief
     @rst
     Each of ChangliNymph's components' local bone -> her merged id, for a mod from before WWMI's merged
     skeleton -- see :cpp:member:`WWMIFixerConfig::sourceVgMaps`
     @endrst
     */
    const std::map<int, std::vector<int>>& changliNymphVgMaps();
}

#endif
