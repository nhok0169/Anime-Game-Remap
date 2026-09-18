#ifndef AGRemapCore_Transparency_H
#define AGRemapCore_Transparency_H

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

     Adjusts the transparency (alpha channel) of a pixel
     @endrst
     */
    class Transparency: public BasePixelTransform {
        public:

            /**
             * @brief
             @rst
             How much to adjust the alpha channel of the pixel. Range from -255 to 255 :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The alpha channel for an image is inclusively bounded from 0 to 255
             @endrst
             */
            int alphaChange;

            /**
             * @brief Constructs a new transparency pixel transform
             *
             * @param alphaChange How much to adjust the alpha channel of the pixel
             */
            explicit Transparency(int alphaChange);

            void transform(Colour &pixel, int x, int y) override;
    };
}

#endif
