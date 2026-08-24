#ifndef AGRemapCore_Transparency_H
#define AGRemapCore_Transparency_H

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
