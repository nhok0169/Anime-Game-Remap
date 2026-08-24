#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace AGRemapCore {

    void BasePixelTransform::operator()(Colour &pixel, int x, int y) {
        transform(pixel, x, y);
    }

    void BasePixelTransform::transform(Colour &, int, int) {}
}
