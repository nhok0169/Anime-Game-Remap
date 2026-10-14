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


#ifndef LYNAE_LOWER_SHADER_H
#define LYNAE_LOWER_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Lynae's lower body pixel shader (``a68c6144f18d6caf``) and the material constants her lower body's draw reads at
     ``ps-cb4``, for her lower body drawn on LynaePeppermint
     :raw-html:`<br />` :raw-html:`<br />`

     Drawn on the skin's slot it renders her lower body as her own draw does. It reads the mod's textures in
     her register order: ``ps-t0`` normal map, ``ps-t1`` mask, ``ps-t2`` material code, ``ps-t3`` diffuse, game textures at ``ps-t4`` to ``ps-t6``, her film foil at ``ps-t7``, her sheer map at ``ps-t8`` and the subsurface ramp at ``ps-t9``
     @endrst
     */
    WWMIFixerConfig::Shader lynaeLowerShader();
}

#endif
