#ifndef AGRemapCore_ColourReplace_H
#define AGRemapCore_ColourReplace_H

#include <optional>

#include "AGRemapCore/model/strategies/texEditors/pixelTransforms/BasePixelTransform.h"
#include "AGRemapCore/model/textures/ColourRange.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BasePixelTransform`

     Replaces a coloured pixel
     @endrst
     */
    class ColourReplace: public BasePixelTransform {
        public:

            /**
             * @brief The colour to fill in
             */
            Colour replaceColour;

            /**
             * @brief
             @rst
             The colours to find to be replaced. If this is ``std::nullopt``, will always replace
             the colour of the pixel
             @endrst
             */
            std::optional<ColourOrRangeSet> coloursToReplace;

            /**
             * @brief Whether to also replace the alpha channel of the original colour
             */
            bool replaceAlpha;

            /**
             * @brief Constructs a new colour-replace pixel transform
             *
             * @param replaceColour The colour to fill in
             * @param coloursToReplace The colours to find to be replaced. If this is
             *      ``std::nullopt``, will always replace the colour of the pixel
             * @param replaceAlpha Whether to also replace the alpha channel of the original colour
             */
            explicit ColourReplace(Colour replaceColour, std::optional<ColourOrRangeSet> coloursToReplace = std::nullopt, bool replaceAlpha = true);

            void transform(Colour &pixel, int x, int y) override;
    };
}

#endif
