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
