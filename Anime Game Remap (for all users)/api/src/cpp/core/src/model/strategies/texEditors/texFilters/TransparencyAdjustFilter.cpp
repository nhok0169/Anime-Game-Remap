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

#include "AGRemapCore/model/strategies/texEditors/texFilters/TransparencyAdjustFilter.h"

#include <utility>

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/Transparency.h"

namespace AGRemapCore {

    TransparencyAdjustFilter::TransparencyAdjustFilter(int alphaChange, std::optional<ColourOrRangeSet> coloursToFilter):
        alphaChange(alphaChange), coloursToFilter(std::move(coloursToFilter)) {}

    void TransparencyAdjustFilter::adjustTransparency(TextureFile &texFile) {
        Transparency pixelTransform(alphaChange);
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

    void TransparencyAdjustFilter::transform(TextureFile &texFile) {
        if (!coloursToFilter.has_value()) {
            adjustTransparency(texFile);
            return;
        }

        Transparency pixelTransform(alphaChange);
        int width = texFile.getWidth();
        int height = texFile.getHeight();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Colour pixel = texFile.getPixel(x, y);

                bool shouldAdjust = false;
                for (const auto &colourOrRange : *coloursToFilter) {
                    if (matchesColourOrRange(colourOrRange, pixel)) {
                        shouldAdjust = true;
                        break;
                    }
                }

                if (shouldAdjust) {
                    pixelTransform.transform(pixel, x, y);
                    texFile.setPixel(x, y, pixel);
                }
            }
        }
    }
}
