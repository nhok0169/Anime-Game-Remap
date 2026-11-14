#ifndef AGRemapCore_TextureRegisters_H
#define AGRemapCore_TextureRegisters_H
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
     Where a character's own draws bind each texture role, per game version -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getTextureRegisterDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     Four index columns -- ``version`` (at position 0), ``name``, ``component`` and ``type`` -- as
     :cpp:class:`Indices` has, so :cpp:class:`ModMappedAssets`'s query methods take three non-version values
     positionally, in that order. A lookup at a version answers with the newest row at or below it
     :raw-html:`<br />` :raw-html:`<br />`

     The slot is the ``component`` (``component0``, ...), the role the ``type`` (``upperDiffuse``), and the value
     the register (``ps-t3``). The same role handed to RabbitFX is the type ``<role>@rabbitfx``, valued with
     RabbitFX's key (``Resource\\RabbitFX\\Diffuse``), and the register another pass of the slot binds it at (an
     outline, named as :cpp:class:`DrawPasses` names the pass) is the type ``<role>@<pass>``
     (``upperDiffuse@other``) :raw-html:`<br />` :raw-html:`<br />`

     A row whose ``name`` is a GAME's (:cpp:func:`GameTypeIdTools::getName`) and whose ``component`` is ``""`` is
     that game's default for every character and slot -- :cpp:func:`ModType::getTextureRegister` falls back to it.
     So a game update that moves a role to another register for every character (GI 5.x moved the diffuse from
     ``ps-t0`` to ``ps-t1``) is one row
     @endrst
     */
    class TextureRegisters: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit TextureRegisters(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
