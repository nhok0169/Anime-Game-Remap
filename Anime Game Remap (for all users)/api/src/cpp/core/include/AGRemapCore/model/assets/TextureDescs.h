#ifndef AGRemapCore_TextureDescs_H
#define AGRemapCore_TextureDescs_H
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
     How the game creates a character's game textures, per game version -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getTextureDescDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     Four index columns -- ``version`` (at position 0), ``name``, ``component`` and ``type`` -- as
     :cpp:class:`Indices` has, so :cpp:class:`ModMappedAssets`'s query methods take three non-version values
     positionally, in that order. A lookup at a version answers with the newest row at or below it
     :raw-html:`<br />` :raw-html:`<br />`

     The ``component`` is ``""`` and the ``type`` is the texture's hash; the value is its description as a frame
     dump's ``.dsc`` gives it, ``format,width,height,mips`` (``BC7_TYPELESS,2048,2048,12``) -- what a texture's own
     file cannot say once a copy has dropped its mips. See :cpp:func:`ModType::getTextureDesc` and
     :cpp:func:`RabbitFX::tagOf`
     @endrst
     */
    class TextureDescs: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit TextureDescs(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
