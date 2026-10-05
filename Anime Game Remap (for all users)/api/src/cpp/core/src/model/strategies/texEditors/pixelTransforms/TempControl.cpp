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

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/TempControl.h"

#include <cmath>

namespace AGRemapCore {

    namespace {
        constexpr double PaintTempIncRedFactor = 0.41;
        constexpr double PaintTempIncBlueFactor = 0.44;
        constexpr double PaintTempDecRedFactor = 0.5;
        constexpr double PaintTempDecBlueFactor = 2.0;
    }

    TempControl::TempControl(double temp):
        temp(temp),
        redFactor_(temp >= 0 ? PaintTempIncRedFactor : PaintTempDecRedFactor),
        blueFactor_(temp >= 0 ? PaintTempIncBlueFactor : PaintTempDecBlueFactor) {}

    void TempControl::transform(Colour &pixel, int, int) {
        pixel.red = Colour::boundColourChannel(static_cast<int>(std::lround(pixel.red + temp * redFactor_ * pixel.red)));
        pixel.blue = Colour::boundColourChannel(static_cast<int>(std::lround(pixel.blue - temp * blueFactor_ * pixel.blue)));
    }
}
