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

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/TintTransform.h"

namespace AGRemapCore {

    TintTransform::TintTransform(int tint): tint(tint) {}

    void TintTransform::transform(Colour &pixel, int, int) {
        pixel.green = Colour::boundColourChannel(pixel.green + tint);
    }
}
