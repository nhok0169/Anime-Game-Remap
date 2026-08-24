#ifndef AGRemapCore_Colour_H
#define AGRemapCore_Colour_H

#include <cstddef>
#include <string>
#include <tuple>

namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class to store data for a colour
     @endrst
     */
    class Colour {
        public:

            /**
             * @brief The red channel for the colour, bounded to [0, 255]
             */
            int red;

            /**
             * @brief The green channel for the colour, bounded to [0, 255]
             */
            int green;

            /**
             * @brief The blue channel for the colour, bounded to [0, 255]
             */
            int blue;

            /**
             * @brief
             @rst
             The transparency (alpha) channel for the colour, bounded to [0, 255] -- 0 = transparent,
             255 = opaque
             @endrst
             */
            int alpha;

            /**
             * @brief Constructs a new colour, defaulting to opaque white
             *
             * @param red The red channel
             * @param green The green channel
             * @param blue The blue channel
             * @param alpha The alpha (transparency) channel
             */
            explicit Colour(int red = 255, int green = 255, int blue = 255, int alpha = 255);

            /**
             * @brief Makes a colour channel value be in between 'min' and 'max'
             *
             * @param val The value of the channel
             * @param min The minimum bound for the colour channel
             * @param max The maximum bound for the colour channel
             *
             * @return The bounded value
             */
            static int boundColourChannel(int val, int min = 0, int max = 255);

            /**
             * @brief Converts a boolean value to a value for a colour channel
             *
             * @param val The boolean value to convert
             * @param min The minimum bound for the colour channel
             * @param max The maximum bound for the colour channel
             *
             * @return 'max' if 'val' is true, otherwise 'min'
             */
            static int boolToColourChannel(bool val, int min = 0, int max = 255);

            /**
             * @brief A hash for the colour, based off #getId
             */
            std::size_t hash() const;

            /**
             * @brief Updates the colour based off 'colourTuple' (red, green, blue, alpha)
             *
             * @param colourTuple The raw values for the colour, in RGBA order
             */
            void fromTuple(const std::tuple<int, int, int, int> &colourTuple);

            /**
             * @brief The tuple representation of the colour, in RGBA order
             */
            std::tuple<int, int, int, int> getTuple() const;

            /**
             * @brief
             @rst
             A unique id for the colour :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The id generated will not correspond to any id generated for a colour range
             @endrst
             */
            std::string getId() const;

            /**
             * @brief Copies the colour value from 'colour'
             *
             * @param colour The colour to copy from
             * @param withAlpha Whether to also copy the alpha channel
             */
            void copy(const Colour &colour, bool withAlpha = true);

            /**
             * @brief Whether 'colour' matches this colour
             *
             * @param colour The colour to check
             */
            bool match(const Colour &colour) const;
    };
}

#endif
