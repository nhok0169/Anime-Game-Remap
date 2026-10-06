#ifndef AGRemapCore_LynaePeppermintSkeleton_H
#define AGRemapCore_LynaePeppermintSkeleton_H

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

#include <map>
#include <vector>


namespace AGRemapCore {
    /**
     * @brief
     @rst
     Each component's local bone -> LynaePeppermint's 3.7 merged id, for a mod from before WWMI's
     merged skeleton -- see :cpp:member:`WWMIFixerConfig::sourceVgMaps`. The skin was never
     renumbered, so this is her 3.7 ``Metadata.json``'s ``vg_map``
     @endrst
     */
    const std::map<int, std::vector<int>>& lynaePeppermintVgMaps();
}

#endif
