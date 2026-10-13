#ifndef AGRemapCore_LynaePeppermintTextures_H
#define AGRemapCore_LynaePeppermintTextures_H

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

#include "AGRemapCore/data/WWMITextureFacts.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What LynaePeppermint's own textures look like -- the hash the game binds each under (the current
     one and the older generation her mods carry), which role each register binds per component, and
     where her game textures are downloaded from :raw-html:`<br />` :raw-html:`<br />`

     Hers, not the remap's: the parser reads them to work out what a mod's files are, whichever
     character the mod is being fixed onto
     @endrst
     */
    WWMITextureFacts lynaePeppermintTextureFacts();
}

#endif
