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


#ifndef LYNAE_UPPER_SHADER_H
#define LYNAE_UPPER_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Lynae's upper body pixel shader (``a175ff3bd01feb29``) and the material constants her upper body's draw reads at
     ``ps-cb4``, for her upper body drawn on LynaePeppermint
     :raw-html:`<br />` :raw-html:`<br />`

     Drawn on the skin's slot it renders her upper body as her own draw does. It reads the mod's textures in
     her register order: ``ps-t0`` normal map, ``ps-t1`` mask, ``ps-t2`` material code, ``ps-t3`` diffuse, the holographic foil at ``ps-t4`` and the character's subsurface ramp at ``ps-t5`` / ``ps-t6``
     @endrst
     */
    WWMIFixerConfig::Shader lynaeUpperShader();
}

#endif
