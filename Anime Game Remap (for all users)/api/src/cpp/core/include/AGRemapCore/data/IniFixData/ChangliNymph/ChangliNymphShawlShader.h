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


#ifndef CHANGLI_NYMPH_SHAWL_SHADER_H
#define CHANGLI_NYMPH_SHAWL_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     ChangliNymph's see-through shawl pixel shader (``43b4972776737988``) and the material constants her
     shawl's draw reads at ``ps-cb4``, for her shawl drawn on Changli
     :raw-html:`<br />` :raw-html:`<br />`

     The same screen-door see-through as :cpp:func:`changliNymphSheerShader`, with the same register
     order: the mod's normal map at ``ps-t0`` and diffuse at ``ps-t1``, a matcap at ``ps-t2``, the dither
     noise at ``ps-t3`` and the subsurface ramp at ``ps-t4``. It reads the per-vertex ``COLOR0`` of the
     coat slot's vertex shader it runs on
     @endrst
     */
    WWMIFixerConfig::Shader changliNymphShawlShader();
}

#endif
