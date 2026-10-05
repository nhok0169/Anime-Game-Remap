#ifndef AGRemapCore_TempControl_H
#define AGRemapCore_TempControl_H

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

     Controls the temperature of a texture file using a modified version of the
     `Simple Image Temperature/Tint Adjust Algorithm`_ such that the colour channels
     increase/decrease linearly with respect to their corresponding pixel value and the user
     selected temperature
     @endrst
     */
    class TempControl: public BasePixelTransform {
        public:

            /**
             * @brief The temperature to set the image. Range from -1 to 1
             */
            double temp;

            /**
             * @brief Constructs a new temperature-control pixel transform
             *
             * @param temp The temperature to set the image
             */
            explicit TempControl(double temp = 0);

            void transform(Colour &pixel, int x, int y) override;

        private:
            double redFactor_;
            double blueFactor_;
    };
}

#endif
