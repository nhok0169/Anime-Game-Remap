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

#include "AGRemapCore/model/strategies/texEditors/texFilters/ColourReplaceFilter.h"

#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/ColourReplace.h"

namespace AGRemapCore {

    ColourReplaceFilter::ColourReplaceFilter(Colour replaceColour, std::optional<ColourOrRangeSet> coloursToReplace, bool replaceAlpha):
        replaceColour(replaceColour), coloursToReplace(std::move(coloursToReplace)), replaceAlpha(replaceAlpha) {}

    void ColourReplaceFilter::transform(TextureFile &texFile) {
        ColourReplace pixelTransform(replaceColour, coloursToReplace, replaceAlpha);
        int width = texFile.getWidth();
        int height = texFile.getHeight();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Colour pixel = texFile.getPixel(x, y);
                pixelTransform.transform(pixel, x, y);
                texFile.setPixel(x, y, pixel);
            }
        }
    }
}
