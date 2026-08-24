#ifndef AGRemapCore_TintTransform_H
#define AGRemapCore_TintTransform_H

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
