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


#ifndef LYNAE_SKIN_CODES_H
#define LYNAE_SKIN_CODES_H

#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Swaps the skin code between Lynae's and LynaePeppermint's body material-code maps :raw-html:`<br />` :raw-html:`<br />`

     Both skins' upper and lower body shaders read an 8-bit code per texel at ``ps-t2``, the low four
     bits a material and the high four flags. Lynae marks bare skin with material 0 and LynaePeppermint
     with material 4, and each uses the other's number for something small (trims, logos). A code map
     carried across unchanged shades the body's skin as cloth: whiter, with grey-lavender shadows,
     beside a face that stays warm. Materials 0 and 4 are exchanged and every other bit is kept, so
     the same filter serves both directions
     @endrst
     */
    TexEditor::Filter lynaeSkinCodeSwap();
}

#endif
