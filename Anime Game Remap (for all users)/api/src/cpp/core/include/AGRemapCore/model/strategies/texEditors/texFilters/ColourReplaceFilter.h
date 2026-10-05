#ifndef AGRemapCore_ColourReplaceFilter_H
#define AGRemapCore_ColourReplaceFilter_H

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

#include <optional>

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"
#include "AGRemapCore/model/textures/ColourRange.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Replaces specific colours in the image -- runs :cpp:class:`ColourReplace` over every pixel
     @endrst
     */
    class ColourReplaceFilter: public BaseTexFilter {
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
             * @brief Constructs a new colour-replace filter
             *
             * @param replaceColour The colour to fill in
             * @param coloursToReplace The colours to find to be replaced. If this is
             *      ``std::nullopt``, will always replace the colour of the pixel
             * @param replaceAlpha Whether to also replace the alpha channel of the original colour
             */
            explicit ColourReplaceFilter(Colour replaceColour, std::optional<ColourOrRangeSet> coloursToReplace = std::nullopt, bool replaceAlpha = true);

            void transform(TextureFile &texFile) override;
    };
}

#endif
