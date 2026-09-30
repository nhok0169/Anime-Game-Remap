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


#ifndef CHISA_PARFAIT_TEXTURES_H
#define CHISA_PARFAIT_TEXTURES_H

#include "AGRemapCore/data/WWMITextureFacts.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What ChisaParfait's own textures look like -- the hash the game binds each under, and which role each
     register binds per component :raw-html:`<br />` :raw-html:`<br />`

     Hers, not the remap's: the same facts identify one of her mods' textures whichever skin it is
     being fixed onto, so they belong to the character. The PARSER reads them to work out what a
     mod's files are
     @endrst
     */
    WWMITextureFacts chisaParfaitTextureFacts();
}

#endif
