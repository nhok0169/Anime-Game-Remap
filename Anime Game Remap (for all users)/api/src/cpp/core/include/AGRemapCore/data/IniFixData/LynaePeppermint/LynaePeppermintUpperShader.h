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


#ifndef LYNAE_PEPPERMINT_UPPER_SHADER_H
#define LYNAE_PEPPERMINT_UPPER_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     LynaePeppermint's upper-body pixel shader (``ed4fe222718a4497``) and the material constants her
     upper body's draw reads at ``ps-cb4``, for her upper body drawn on Lynae
     :raw-html:`<br />` :raw-html:`<br />`

     It writes her torso, arms and swimsuit top into the G-buffer with her own materials. It reads the
     mod's textures in her upper body's register order (``ps-t0`` normal map, ``ps-t1`` mask, ``ps-t2``
     material code, ``ps-t3`` diffuse), her noise at ``ps-t4``, her foil at ``ps-t5`` and the
     character's subsurface ramp at ``ps-t6``
     @endrst
     */
    WWMIFixerConfig::Shader lynaePeppermintUpperShader();
}

#endif
