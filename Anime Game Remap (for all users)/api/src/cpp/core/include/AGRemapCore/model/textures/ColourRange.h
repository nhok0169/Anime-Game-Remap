#ifndef AGRemapCore_ColourRange_H
#define AGRemapCore_ColourRange_H

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class to store a range for a colour
     @endrst
     */
    class ColourRange {
        public:

            /**
             * @brief The minimum range for the RGBA values
             */
            Colour min;

            /**
             * @brief The maximum range for the RGBA values
             */
            Colour max;

            /**
             * @brief Constructs a new colour range
             *
             * @param min The minimum range for the RGBA values
             * @param max The maximum range for the RGBA values
             */
            ColourRange(Colour min, Colour max);

            /**
             * @brief A hash for the colour range, based off #getId
             */
            std::size_t hash() const;

            /**
             * @brief
             @rst
             A unique id for the colour range :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The id generated will not correspond to any id generated for a single colour
             @endrst
             */
            std::string getId() const;

            /**
             * @brief Whether 'colour' is within the colour range
             *
             * @param colour The colour to check
             */
            bool match(const Colour &colour) const;
    };

    /**
     * @brief A single colour, or a range of colours
     */
    using ColourOrRange = std::variant<Colour, ColourRange>;

    /**
     * @brief A collection of colours/colour ranges, used by filters to match against a pixel
     */
    using ColourOrRangeSet = std::vector<ColourOrRange>;

    /**
     * @brief Whether 'colour' matches 'colourOrRange' (a single colour needs an exact match; a
     *      colour range needs the colour to fall within it)
     *
     * @param colourOrRange The single colour or colour range to check against
     * @param colour The colour to check
     */
    bool matchesColourOrRange(const ColourOrRange &colourOrRange, const Colour &colour);
}

#endif
