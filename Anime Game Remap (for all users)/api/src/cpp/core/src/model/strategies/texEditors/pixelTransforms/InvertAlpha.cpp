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

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/InvertAlpha.h"

namespace AGRemapCore {

    void InvertAlpha::transform(Colour &pixel, int, int) {
        // See the class doc comment -- '0 -' (not '255 -') matches the pure-Python original exactly.
        pixel.alpha = 0 - pixel.alpha;
    }
}
