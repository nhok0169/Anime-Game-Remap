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

#include "AGRemapCore/model/strategies/texEditors/texFilters/GammaFilter.h"

#include <vector>

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/CorrectGamma.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    GammaFilter::GammaFilter(double gamma): gamma(gamma) {}

    void GammaFilter::transform(TextureFile &texFile) {
        CorrectGamma correctGamma(gamma);
        int width = texFile.getWidth();
        int height = texFile.getHeight();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Colour pixel = texFile.getPixel(x, y);
                correctGamma.transform(pixel, x, y);
                texFile.setPixel(x, y, pixel);
            }
        }
    }
}
