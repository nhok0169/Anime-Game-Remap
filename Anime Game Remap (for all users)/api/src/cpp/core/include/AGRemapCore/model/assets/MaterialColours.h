#ifndef AGRemapCore_MaterialColours_H
#define AGRemapCore_MaterialColours_H
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

#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     The colours a character's multi-channel material masks mark each material with, per game version -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getMaterialColourDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     Four index columns -- ``version`` (at position 0), ``name``, ``component`` and ``type`` -- as
     :cpp:class:`Indices` has, so :cpp:class:`ModMappedAssets`'s query methods take three non-version values
     positionally, in that order. A lookup at a version answers with the newest row at or below it
     :raw-html:`<br />` :raw-html:`<br />`

     The ``component`` is the draw slot (``component3``) and the ``type`` names a mask role and a material
     (``torsoMask.skin``); the value is the :cpp:class:`Colour` the mask marks it with, written by
     :cpp:func:`MaterialColours::valueOf` and read by :cpp:func:`ModType::getMaterialColour` :raw-html:`<br />` :raw-html:`<br />`

     A colour every character shares (a flat normal map, white) is a :cpp:struct:`Colours` constant instead, which a
     row may be written with
     @endrst
     */
    class MaterialColours: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit MaterialColours(std::unordered_map<std::string, std::vector<std::string>> map = {});

            /**
             * @brief A colour as the table holds it: ``r,g,b,a``
             *
             * @param colour The colour
             */
            static std::string valueOf(const Colour& colour);

            /**
             * @brief The colour a value of the table holds (#valueOf read back)
             *
             * @param value The value, ``r,g,b,a``
             *
             * @throws std::invalid_argument When the value is not four comma-separated channels
             */
            static Colour colourOf(const std::string& value);
    };
}

#endif
