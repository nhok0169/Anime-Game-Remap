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


#ifndef CHANGLI_NYMPH_TEXTURES_H
#define CHANGLI_NYMPH_TEXTURES_H

#include "AGRemapCore/data/WWMITextureFacts.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What ChangliNymph's (Changli's Laurel Nymph skin's) own textures look like -- the hashes the game binds each under (WWMI-Assets' and the
     live ones the game binds at its highest texture quality), which role each register binds per
     component, and where her game textures are downloaded from :raw-html:`<br />` :raw-html:`<br />`

     Hers, not the remap's: the parser reads them to work out what a mod's files are, whichever skin
     the mod is being fixed onto
     @endrst
     */
    WWMITextureFacts changliNymphTextureFacts();
}

#endif
