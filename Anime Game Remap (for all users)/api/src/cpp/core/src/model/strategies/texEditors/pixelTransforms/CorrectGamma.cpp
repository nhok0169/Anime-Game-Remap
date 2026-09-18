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

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/CorrectGamma.h"

#include <cmath>

#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    CorrectGamma::CorrectGamma(double gamma): gamma(gamma) {}

    int CorrectGamma::correctGamma(int pixelValue, double gamma) {
        double normalized = static_cast<double>(pixelValue) / 255.0;
        double corrected = std::pow(normalized, 1.0 / gamma) * 255.0;
        return Colour::boundColourChannel(static_cast<int>(std::lround(corrected)));
    }

    void CorrectGamma::transform(Colour &pixel, int, int) {
        pixel.red = correctGamma(pixel.red, gamma);
        pixel.green = correctGamma(pixel.green, gamma);
        pixel.blue = correctGamma(pixel.blue, gamma);
    }
}
