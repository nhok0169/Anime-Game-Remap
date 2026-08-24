#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/ColourReplace.h"

#include <utility>

namespace AGRemapCore {

    ColourReplace::ColourReplace(Colour replaceColour, std::optional<ColourOrRangeSet> coloursToReplace, bool replaceAlpha):
        replaceColour(replaceColour), coloursToReplace(std::move(coloursToReplace)), replaceAlpha(replaceAlpha) {}

    void ColourReplace::transform(Colour &pixel, int, int) {
        if (!coloursToReplace.has_value()) {
            pixel.copy(replaceColour, replaceAlpha);
            return;
        }

        for (const auto &colourOrRange : *coloursToReplace) {
            if (matchesColourOrRange(colourOrRange, pixel)) {
                pixel.copy(replaceColour, replaceAlpha);
                return;
            }
        }
    }
}
