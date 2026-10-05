#ifndef AGRemapCore_InvertAlpha_H
#define AGRemapCore_InvertAlpha_H

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

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BasePixelTransform`

     Inverts the alpha channel of a pixel

     .. warning::
        This computes the new alpha as ``0 - alpha`` (the *minimum* colour value minus the pixel's
        alpha) rather than ``255 - alpha`` (the *maximum* minus the pixel's alpha) that a "true"
        alpha invert would use -- unlike :cpp:class:`AGRemapCore::TexEditor`'s sibling whole-image
        ``InvertAlphaFilter``, which does use ``255 - alpha``. Use that filter when a true alpha
        invert is wanted
     @endrst
     */
    class InvertAlpha: public BasePixelTransform {
        public:
            void transform(Colour &pixel, int x, int y) override;
    };
}

#endif
