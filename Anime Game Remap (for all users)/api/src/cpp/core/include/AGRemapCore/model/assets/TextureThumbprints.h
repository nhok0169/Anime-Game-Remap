#ifndef AGRemapCore_TextureThumbprints_H
#define AGRemapCore_TextureThumbprints_H
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


namespace AGRemapCore {
    /**
     * @brief
     @rst
     What a character's game textures look like, as small thumbprints, per game version -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getTextureThumbprintDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     Four index columns -- ``version`` (at position 0), ``name``, ``component`` and ``type`` -- as
     :cpp:class:`Indices` has, so :cpp:class:`ModMappedAssets`'s query methods take three non-version values
     positionally, in that order. A lookup at a version answers with the newest row at or below it
     :raw-html:`<br />` :raw-html:`<br />`

     The ``component`` is ``""`` and the ``type`` is the texture's hash; the value is its thumbprint -- a 16 x 16
     grayscale box average of the decoded texture, each pixel's ``(r + g + b) / 3`` -- as 256 bytes in hex, row by row.
     A fix places a mod file that carries no hash by correlating its own thumbprint against these. See
     :cpp:func:`ModType::getTextureThumbprints` and :cpp:member:`WWMITextureFacts::textureThumbprints`
     @endrst
     */
    class TextureThumbprints: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit TextureThumbprints(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
