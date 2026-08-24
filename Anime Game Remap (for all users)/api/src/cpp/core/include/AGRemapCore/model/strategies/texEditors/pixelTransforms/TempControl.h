#ifndef AGRemapCore_TempControl_H
#define AGRemapCore_TempControl_H

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
