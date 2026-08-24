#ifndef AGRemapCore_InvertAlpha_H
#define AGRemapCore_InvertAlpha_H

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BasePixelTransform`

     Inverts the alpha channel of a pixel

     .. warning::
        Ported byte-for-byte from the pure-Python original, which computes the new alpha as
        ``0 - alpha`` (the *minimum* colour value minus the pixel's alpha) rather than
        ``255 - alpha`` (the *maximum* minus the pixel's alpha) that a "true" alpha invert would
        use -- unlike :cpp:class:`AGRemapCore::TexEditor`'s sibling whole-image
        ``InvertAlphaFilter``, which does use ``255 - alpha``. This looks like a pre-existing bug in
        the original, not intentional, but it's preserved here rather than silently corrected
     @endrst
     */
    class InvertAlpha: public BasePixelTransform {
        public:
            void transform(Colour &pixel, int x, int y) override;
    };
}

#endif
