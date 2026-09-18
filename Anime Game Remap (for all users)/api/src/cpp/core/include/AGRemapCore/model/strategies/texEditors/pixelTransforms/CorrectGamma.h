#ifndef AGRemapCore_CorrectGamma_H
#define AGRemapCore_CorrectGamma_H

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

     Performs a `Gamma Correction`_ on an individual pixel using the following simple power-law
     relationship:

     .. code-block::

        V_out = V_in ^ (1 / gamma)

     Where ``V_out`` is the perceived brightness by human eyes while ``V_in`` is the actual
     brightness of the image. Higher #gamma values make the image look brighter and less
     saturated; lower #gamma values make the image look darker and more saturated. The alpha
     channel is left untouched
     @endrst
     */
    class CorrectGamma: public BasePixelTransform {
        public:

            /**
             * @brief The luminance parameter for how bright humans perceive the image
             */
            double gamma;

            /**
             * @brief Constructs a new gamma-correction pixel transform
             *
             * @param gamma The luminance parameter for how bright humans perceive the image
             */
            explicit CorrectGamma(double gamma);

            /**
             * @brief The equation for the gamma correction done at every colour channel pixel
             *
             * @param pixelValue The value of the pixel for some colour channel, in [0, 255]
             * @param gamma The luminance parameter for how bright humans perceive the image
             *
             * @return The gamma-corrected pixel value, bounded to [0, 255]
             */
            static int correctGamma(int pixelValue, double gamma);

            void transform(Colour &pixel, int x, int y) override;
    };
}

#endif
