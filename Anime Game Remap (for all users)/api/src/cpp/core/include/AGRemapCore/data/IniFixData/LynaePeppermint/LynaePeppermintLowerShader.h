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


#ifndef LYNAE_PEPPERMINT_LOWER_SHADER_H
#define LYNAE_PEPPERMINT_LOWER_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     LynaePeppermint's lower-body pixel shader (``0a52e215e81518f3``) and the material constants her
     lower body's draw reads at ``ps-cb4``, for her lower body drawn on Lynae
     :raw-html:`<br />` :raw-html:`<br />`

     It writes her legs, hips and shorts into the G-buffer, and draws the holographic parts of her
     outfit with a foil the target's own lower-body shader does not have. It reads the mod's textures
     in her lower body's register order (``ps-t0`` normal map, ``ps-t1`` mask, ``ps-t2`` material
     code, ``ps-t3`` diffuse), shared game textures at ``ps-t4`` to ``ps-t6``, her noise at
     ``ps-t7``, the holographic foil at ``ps-t8`` and the character's subsurface ramp at ``ps-t9``
     @endrst
     */
    WWMIFixerConfig::Shader lynaePeppermintLowerShader();
}

#endif
