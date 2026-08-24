#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/Transparency.h"

namespace AGRemapCore {

    Transparency::Transparency(int alphaChange): alphaChange(alphaChange) {}

    void Transparency::transform(Colour &pixel, int, int) {
        pixel.alpha = Colour::boundColourChannel(pixel.alpha + alphaChange);
    }
}
