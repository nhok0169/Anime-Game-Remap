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


#ifndef CHANGLI_UPPER_SHADER_H
#define CHANGLI_UPPER_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Changli's upper-body pixel shader (``9d4c7d9618ef7f45``) and the material constants her upper body's
     draw reads at ``ps-cb4``, as an emissive overlay for her flame feathers' glow on ChangliNymph
     :raw-html:`<br />` :raw-html:`<br />`

     A part of hers glows where its diffuse's alpha is above 230 of 255 (her emissive is a colour times
     ``saturate((alpha - 0.9) / 0.1)``). This copy discards
     every pixel below that and writes the part's diffuse colour, doubled, as the pixel's emissive, so it is
     drawn over a surface another shader has already drawn, with that shader's other render targets kept. It
     reads the mod's normal map, mask, diffuse and detail map at ``ps-t0`` .. ``ps-t3``, her flame ramp at
     ``ps-t4``, a game texture at ``ps-t5`` and the subsurface ramp at ``ps-t6``
     @endrst
     */
    WWMIFixerConfig::Shader changliUpperShader();
}

#endif
