#ifndef AGRemapCore_HighlightShadow_H
#define AGRemapCore_HighlightShadow_H

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

     A filter that approximates the adjustment of the shadow/highlight of an image
     @endrst
     */
    class HighlightShadow: public BasePixelTransform {
        public:

            /**
             * @brief The amount of highlight to apply to the pixel. Range from -1 to 1, and 0 = no change
             */
            double highlight;

            /**
             * @brief The amount of shadow to apply to the pixel. Range from -1 to 1, and 0 = no change
             */
            double shadow;

            /**
             * @brief Constructs a new highlight/shadow pixel transform
             *
             * @param highlight The amount of highlight to apply to the pixel
             * @param shadow The amount of shadow to apply to the pixel
             */
            explicit HighlightShadow(double highlight = 0, double shadow = 0);

            void transform(Colour &pixel, int x, int y) override;
    };
}

#endif
