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


#ifndef CHANGLI_TATTOO_SHADER_H
#define CHANGLI_TATTOO_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Changli's chest flame tattoo pixel shader (``bb1c5ba6e1be6395``) and the material constants its draw
     reads, for her tattoo drawn on ChangliNymph
     :raw-html:`<br />` :raw-html:`<br />`

     The shader cuts the tattoo's shape out of its glow texture's alpha and discards the rest of the quad.
     Its copy is made for ChangliNymph's body draw: it takes the ``TEXCOORD4`` / ``TEXCOORD5`` inputs it
     does not read (that draw's vertex shader writes them before the inputs it does read), and its material
     constants are at ``ps-cb1`` with the per-object constants at ``ps-cb2``, where that draw keeps them. It
     reads the glow texture at ``ps-t0`` and two game textures at ``ps-t1`` / ``ps-t2``
     @endrst
     */
    WWMIFixerConfig::Shader changliTattooShader();
}

#endif
