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

#include "AGRemapCore/model/strategies/texEditors/texFilters/InvertAlphaFilter.h"

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    void InvertAlphaFilter::transform(TextureFile &texFile) {
        int width = texFile.getWidth();
        int height = texFile.getHeight();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Colour pixel = texFile.getPixel(x, y);
                pixel.alpha = 255 - pixel.alpha;
                texFile.setPixel(x, y, pixel);
            }
        }
    }
}
