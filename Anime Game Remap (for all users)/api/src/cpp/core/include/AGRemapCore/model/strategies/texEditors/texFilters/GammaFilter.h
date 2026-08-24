#ifndef AGRemapCore_GammaFilter_H
#define AGRemapCore_GammaFilter_H

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Performs a `Gamma Correction`_ on the texture file, using the following simple power-law
     relationship, applied independently to every pixel's R/G/B channels (the alpha channel is left
     untouched):

     .. code-block::

        V_out = V_in ^ (1 / gamma)

     Where ``V_out`` is the perceived brightness by human eyes while ``V_in`` is the actual
     brightness of the image. Higher #gamma values make the image look brighter and less saturated;
     lower #gamma values make the image look darker and more saturated
     @endrst
     */
    class GammaFilter: public BaseTexFilter {
        public:

            /**
             * @brief The luminance parameter for how bright humans perceive the image
             */
            double gamma;

            /**
             * @brief Constructs a new gamma filter
             *
             * @param gamma The luminance parameter for how bright humans perceive the image
             */
            explicit GammaFilter(double gamma);

            void transform(TextureFile &texFile) override;
    };
}

#endif
