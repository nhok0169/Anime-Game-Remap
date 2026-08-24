#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/InvertAlpha.h"

namespace AGRemapCore {

    void InvertAlpha::transform(Colour &pixel, int, int) {
        // See the class doc comment -- '0 -' (not '255 -') matches the pure-Python original exactly.
        pixel.alpha = 0 - pixel.alpha;
    }
}
