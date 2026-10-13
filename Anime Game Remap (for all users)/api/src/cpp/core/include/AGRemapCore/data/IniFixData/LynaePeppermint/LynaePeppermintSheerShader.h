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


#ifndef LYNAE_PEPPERMINT_SHEER_SHADER_H
#define LYNAE_PEPPERMINT_SHEER_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     LynaePeppermint's see-through jacket pixel shader (``0b6e3ef7b7c30cd3``) and the material constants
     her jacket's draw reads at ``ps-cb4``, for her shirt, jacket and shoes drawn on Lynae
     :raw-html:`<br />` :raw-html:`<br />`

     The shader writes the shirt into the G-buffer marked as sheer, and the game's lighting then
     shows the body drawn underneath through it. It reads the mod's textures in her jacket's register
     order (``ps-t0`` normal map, ``ps-t1`` mask, ``ps-t2`` diffuse, ``ps-t3`` material code), her
     noise texture at ``ps-t4``, the holographic foil at ``ps-t5`` and the character's subsurface ramp
     at ``ps-t6``
     @endrst
     */
    WWMIFixerConfig::Shader lynaePeppermintSheerShader();
}

#endif
