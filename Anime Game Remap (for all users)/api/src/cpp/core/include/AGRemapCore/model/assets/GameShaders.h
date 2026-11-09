#ifndef AGRemapCore_GameShaders_H
#define AGRemapCore_GameShaders_H
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
     The game pixel shaders a fix draws a character's parts with, per game version -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getGameShaderDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     Four index columns -- ``version`` (at position 0), ``name``, ``component`` and ``type`` -- as
     :cpp:class:`Indices` has, so :cpp:class:`ModMappedAssets`'s query methods take three non-version values
     positionally, in that order. A lookup at a version answers with the newest row at or below it
     :raw-html:`<br />` :raw-html:`<br />`

     The ``component`` is ``""`` and the ``type`` names the shader (``LynaeUpper``): its value is the shader's
     decompiled HLSL; ``<shader>.<register>`` (eg. ``LynaeUpper.ps-cb4``) is a constant buffer its draw read at that
     register, as 32-bit words in hex, comma-separated :raw-html:`<br />` :raw-html:`<br />`

     Game shaders change on updates faster than anything else a remap reads, so each is filed under the game
     version it was read from
     @endrst
     */
    class GameShaders: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit GameShaders(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
