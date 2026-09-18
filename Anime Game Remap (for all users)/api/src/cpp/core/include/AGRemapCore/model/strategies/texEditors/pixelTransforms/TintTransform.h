#ifndef AGRemapCore_TintTransform_H
#define AGRemapCore_TintTransform_H

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

     Controls the tint of a texture file using the `Simple Image Temperature/Tint Adjust Algorithm`_
     @endrst
     */
    class TintTransform: public BasePixelTransform {
        public:

            /**
             * @brief The tint to set the image. Range from -100 to 100
             */
            int tint;

            /**
             * @brief Constructs a new tint pixel transform
             *
             * @param tint The tint to set the image
             */
            explicit TintTransform(int tint = 0);

            void transform(Colour &pixel, int x, int y) override;
    };
}

#endif
