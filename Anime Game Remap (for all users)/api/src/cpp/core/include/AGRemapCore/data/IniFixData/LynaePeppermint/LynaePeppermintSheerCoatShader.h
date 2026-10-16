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


#ifndef LYNAE_PEPPERMINT_SHEER_COAT_SHADER_H
#define LYNAE_PEPPERMINT_SHEER_COAT_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     LynaePeppermint's see-through coat pixel shader (``4470be479e212ca1``) and the material constants
     her coat's draw reads at ``ps-cb4``, for her coat drawn on Lynae
     :raw-html:`<br />` :raw-html:`<br />`

     Her coat is drawn as her shirt is: a solid pass, then this one, which writes the coat into the
     G-buffer marked as sheer where its material says so, and the game's lighting then shows the body
     drawn underneath through it. It reads the mod's textures in her coat's register order (``ps-t0``
     normal map, ``ps-t1`` mask, ``ps-t2`` material code, ``ps-t3`` diffuse), her noise texture at
     ``ps-t4``, the coat's foil at ``ps-t5`` and the character's subsurface ramp at ``ps-t6``
     @endrst
     */
    WWMIFixerConfig::Shader lynaePeppermintSheerCoatShader();
}

#endif
