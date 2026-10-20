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


#ifndef LYNAE_PEPPERMINT_SOLID_SHADER_H
#define LYNAE_PEPPERMINT_SOLID_SHADER_H

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     LynaePeppermint's SOLID jacket pixel shader (``efc2690a4acd3c11``), the pass her jacket's draw makes before
     its see-through one, for the opaque parts of her shirt, jacket and shoes drawn on Lynae away from the body
     :raw-html:`<br />` :raw-html:`<br />`

     It reads the same registers and material constants as :cpp:func:`lynaePeppermintSheerShader`, and its
     ``constantBuffers`` are empty: the pass it draws in takes the see-through shader's ``ps-cb4``
     @endrst
     */
    WWMIFixerConfig::Shader lynaePeppermintSolidShader();
}

#endif
