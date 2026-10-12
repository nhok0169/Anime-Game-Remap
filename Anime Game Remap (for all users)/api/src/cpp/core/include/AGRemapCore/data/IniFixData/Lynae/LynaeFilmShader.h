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


#ifndef LYNAE_FILM_SHADER_H
#define LYNAE_FILM_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Lynae's jacket pixel shader (``404f5464fdffc665``) and the material constants her jacket's draw
     reads at ``ps-cb4``, for the draws of a Lynae mod that LynaePeppermint's slots cannot draw
     :raw-html:`<br />` :raw-html:`<br />`

     Her jacket shader draws a material code carrying flag ``0x10`` as a see-through holographic film.
     No slot of LynaePeppermint has that material; with this shader those draws render as they do on
     Lynae, and the textures they read are the mod's own, in her jacket's register order (``ps-t0``
     normal map, ``ps-t1`` mask, ``ps-t2`` material code, ``ps-t3`` diffuse, ``ps-t4`` her foil). Her
     shader also reads the character's subsurface ramp at ``ps-t5``, which the target's draw has at
     ``ps-t6``
     @endrst
     */
    WWMIFixerConfig::DrawReroute::Shader lynaeFilmShader();
}

#endif
