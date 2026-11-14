#ifndef AGRemapCore_Colours_H
#define AGRemapCore_Colours_H

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

#include "AGRemapCore/model/textures/Colour.h"
#include "AGRemapCore/model/textures/ColourRange.h"

namespace AGRemapCore {

    /**
     * @brief
     @rst
     Colours the texture edits share: a flat normal map or light map a fix creates or replaces a band with, and white
     :raw-html:`<br />` :raw-html:`<br />`

     A colour that is ONE character's game data (the skin code in its material mask, say) is not here: it is a row of a
     versioned asset table (:cpp:class:`MaterialColours`)
     @endrst
     */
    struct Colours {
        /**
         * @brief Opaque white
         */
        static inline const Colour White = Colour(255, 255, 255, 255);

        /**
         * @brief The green a light map's flat (no-band) texels carry
         */
        static inline const Colour LightMapGreen = Colour(0, 128, 0, 255);

        /**
         * @brief The low end of the light map's green band (see :cpp:member:`ColourRanges::LightMapGreen`)
         */
        static inline const Colour LightMapGreenMin = Colour(0, 125, 0, 0);

        /**
         * @brief The high end of the light map's green band (see :cpp:member:`ColourRanges::LightMapGreen`)
         */
        static inline const Colour LightMapGreenMax = Colour(50, 160, 50, 255);

        /**
         * @brief A flat normal map in the yellow encoding (GI before 5.x)
         */
        static inline const Colour NormalMapYellow = Colour(128, 128, 0, 255);

        /**
         * @brief A flat normal map in the blue (tangent space) encoding
         */
        static inline const Colour NormalMapBlue = Colour(128, 128, 255, 255);

        /**
         * @brief A flat normal map in the purple encoding, as GI 5.x writes its normal maps
         */
        static inline const Colour NormalMapPurple1 = Colour(128, 98, 128, 255);

        /**
         * @brief The second purple a GI 5.x normal map is flat at (a lighter green channel)
         */
        static inline const Colour NormalMapPurple2 = Colour(128, 114, 128, 255);

        /**
         * @brief The low end of the purple normal maps' bands (see :cpp:member:`ColourRanges::NormalMapPurple1`)
         */
        static inline const Colour NormalMapPurpleMin = Colour(128, 0, 128, 255);
    };

    /**
     * @brief The colour bands the texture edits share, each a :cpp:class:`ColourRange` of :cpp:struct:`Colours`
     */
    struct ColourRanges {
        /**
         * @brief A light map's green band
         */
        static inline const ColourRange LightMapGreen = ColourRange(Colours::LightMapGreenMin, Colours::LightMapGreenMax);

        /**
         * @brief A purple normal map's band, up to :cpp:member:`Colours::NormalMapPurple1`
         */
        static inline const ColourRange NormalMapPurple1 = ColourRange(Colours::NormalMapPurpleMin, Colours::NormalMapPurple1);

        /**
         * @brief A purple normal map's band, up to :cpp:member:`Colours::NormalMapPurple2`
         */
        static inline const ColourRange NormalMapPurple2 = ColourRange(Colours::NormalMapPurpleMin, Colours::NormalMapPurple2);
    };
}

#endif
