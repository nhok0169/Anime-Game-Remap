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
