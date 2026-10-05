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

#include "AGRemapCore/data/ModDataAssets.h"


namespace AGRemapCore {
    const std::shared_ptr<VGRemaps>& ModDataAssets::vgRemaps() {
        // Function-local static: the C++ equivalent of the pure-Python original's DeferredEnum --
        // built once, on first access, thread-safely. Held by shared_ptr because that is what
        // ModType::vgRemaps holds, and every ModType falling back to this one shares it.
        static const std::shared_ptr<VGRemaps> remaps = std::make_shared<VGRemaps>();
        return remaps;
    }
}
