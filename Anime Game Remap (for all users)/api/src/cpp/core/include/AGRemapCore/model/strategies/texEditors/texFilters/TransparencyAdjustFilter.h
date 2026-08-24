#ifndef AGRemapCore_TransparencyAdjustFilter_H
#define AGRemapCore_TransparencyAdjustFilter_H

#include <optional>

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "AGRemapCore/model/textures/ColourRange.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Adjusts the transparency (alpha channel) for an image, using :cpp:class:`Transparency` for the
     underlying per-pixel adjustment
     @endrst
     */
    class TransparencyAdjustFilter: public BaseTexFilter {
        public:

            /**
             * @brief
             @rst
             How much to adjust the alpha channel of each pixel. Range from -255 to 255 :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The alpha channel for an image is inclusively bounded from 0 to 255
             @endrst
             */
            int alphaChange;

            /**
             * @brief
             @rst
             The specific colours to have their transparency adjusted. If this is
             ``std::nullopt``, will adjust the transparency for the entire image
             @endrst
             */
            std::optional<ColourOrRangeSet> coloursToFilter;

            /**
             * @brief Constructs a new transparency-adjust filter
             *
             * @param alphaChange How much to adjust the alpha channel of each pixel
             * @param coloursToFilter The specific colours to have their transparency adjusted. If
             *      this is ``std::nullopt``, will adjust the transparency for the entire image
             */
            explicit TransparencyAdjustFilter(int alphaChange, std::optional<ColourOrRangeSet> coloursToFilter = std::nullopt);

            /**
             * @brief Adjusts the transparency for the entire image
             *
             * @param texFile The texture to be edited
             */
            void adjustTransparency(TextureFile &texFile);

            void transform(TextureFile &texFile) override;
    };
}

#endif
