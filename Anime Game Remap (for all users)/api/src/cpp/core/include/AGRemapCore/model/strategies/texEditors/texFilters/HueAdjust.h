#ifndef AGRemapCore_HueAdjust_H
#define AGRemapCore_HueAdjust_H

#include "AGRemapCore/model/strategies/texEditors/texFilters/BaseTexFilter.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseTexFilter`

     Adjusts the hue of a texture file, by converting to HSV (H/S/V each encoded as a single byte,
     0-255, matching `Pillow`_'s own ``"HSV"`` image mode convention), adjusting the H channel, and
     converting back to RGBA :raw-html:`<br />` :raw-html:`<br />`

     .. warning::
        Ported byte-for-byte from the pure-Python original's own #adjustedHue, which mixes
        byte-scaled (0-255) and degree-scaled (0-360) arithmetic on the same value -- eg. adding a
        [-180, 180]-ranged #hue directly onto a 0-255-ranged H byte, and wrapping an overflow past
        360 with ``360 - result`` (which produces a *negative* result, not a wrapped-around one).
        This looks like a pre-existing bug in the original, not intentional, but it's preserved here
        (bounded to [0, 255] before being used as an H byte, since a raw negative/out-of-range value
        has no well-defined meaning as one) rather than silently corrected
     @endrst
     */
    class HueAdjust: public BaseTexFilter {
        public:

            /**
             * @brief The hue to adjust the image. Value is from -180 to 180
             */
            int hue;

            /**
             * @brief Constructs a new hue-adjust filter
             *
             * @param hue The hue to adjust the image
             */
            explicit HueAdjust(int hue);

            /**
             * @brief
             @rst
             Adjusts a single H byte (0-255) -- see the class doc comment for why this reproduces
             the original's own byte/degree-scale mismatch rather than a "correct" hue wraparound
             @endrst
             *
             * @param hue The current H byte value that has not been adjusted yet
             *
             * @return The adjusted H byte value, bounded to [0, 255]
             */
            int adjustedHue(int hue) const;

            void transform(TextureFile &texFile) override;
    };
}

#endif
