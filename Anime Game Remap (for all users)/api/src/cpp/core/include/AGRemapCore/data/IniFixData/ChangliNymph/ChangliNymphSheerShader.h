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


#ifndef CHANGLI_NYMPH_SHEER_SHADER_H
#define CHANGLI_NYMPH_SHEER_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     ChangliNymph's see-through dress pixel shader (``d5572b1cbbba07a4``) and the material constants her
     dress's draw reads at ``ps-cb4``, for her sheer dress drawn on Changli
     :raw-html:`<br />` :raw-html:`<br />`

     The shader is see-through by a screen-door dither: it discards each pixel whose opacity (the diffuse's
     alpha) falls under a noise threshold that moves every frame, and the game's temporal anti-aliasing
     blends the result. It reads the mod's normal map at ``ps-t0`` and diffuse at ``ps-t1``, a matcap at
     ``ps-t2``, the dither noise at ``ps-t3`` and the subsurface ramp at ``ps-t4``. Its ``COLOR0`` input
     is a constant 0 (her dress's value on every vertex): the vertex shader of the Changli draw it runs on
     passes no colour
     @endrst
     */
    WWMIFixerConfig::Shader changliNymphSheerShader();
}

#endif
