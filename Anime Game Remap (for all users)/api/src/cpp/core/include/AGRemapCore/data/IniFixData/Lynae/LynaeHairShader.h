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


#ifndef LYNAE_HAIR_SHADER_H
#define LYNAE_HAIR_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Lynae's hair pixel shader (``d4252cf5b68968eb``) and the material constants her hair's draw reads at
     ``ps-cb4``, for her hair drawn on LynaePeppermint
     :raw-html:`<br />` :raw-html:`<br />`

     Drawn on the skin's slot it renders her hair as her own draw does. It reads the mod's textures in
     her register order: ``ps-t0`` mask, ``ps-t1`` diffuse, ``ps-t2`` hair ramp, a game texture at ``ps-t3``, the shade ramp at ``ps-t4``, the strand map at ``ps-t5`` and the subsurface ramp at ``ps-t6``
     @endrst
     */
    WWMIFixerConfig::Shader lynaeHairShader();
}

#endif
