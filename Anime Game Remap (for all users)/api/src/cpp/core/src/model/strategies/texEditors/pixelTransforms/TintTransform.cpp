#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/TintTransform.h"

namespace AGRemapCore {

    TintTransform::TintTransform(int tint): tint(tint) {}

    void TintTransform::transform(Colour &pixel, int, int) {
        pixel.green = Colour::boundColourChannel(pixel.green + tint);
    }
}
