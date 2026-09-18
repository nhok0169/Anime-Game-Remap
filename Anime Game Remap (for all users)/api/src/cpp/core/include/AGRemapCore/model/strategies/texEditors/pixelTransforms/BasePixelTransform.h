#ifndef AGRemapCore_BasePixelTransform_H
#define AGRemapCore_BasePixelTransform_H

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

#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class for transforming a pixel in a texture file
     @endrst
     */
    class BasePixelTransform {
        public:
            virtual ~BasePixelTransform() = default;

            /**
             * @brief Calls #transform for this pixel transform
             *
             * @param pixel The pixel to be edited
             * @param x x-coordinate of the pixel
             * @param y y-coordinate of the pixel
             */
            void operator()(Colour &pixel, int x, int y);

            /**
             * @brief Applies a transformation to 'pixel'. No-op by default
             *
             * @param pixel The pixel to be edited
             * @param x x-coordinate of the pixel
             * @param y y-coordinate of the pixel
             */
            virtual void transform(Colour &pixel, int x, int y);
    };
}

#endif
