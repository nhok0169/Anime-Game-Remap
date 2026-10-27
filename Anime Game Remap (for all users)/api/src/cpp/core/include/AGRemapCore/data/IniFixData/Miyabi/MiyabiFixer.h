#ifndef AGRemapCore_MiyabiFixer_H
#define AGRemapCore_MiyabiFixer_H

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

#include "AGRemapCore/data/IniFixData/ZZMIFixer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The Miyabi -> MiyabiBlossom (Hoshimi Miyabi: Dignified Blossom) remap, ZZZ 3.2 -- the config
     :cpp:func:`makeZZMIParser` and :cpp:func:`makeZZMIFixer` build Miyabi's parser and her four fixers
     from: her ``Body`` (her outfit) onto the skin's ``Dress``, her ``Legs`` (bare limbs) onto its
     ``Body``, and ``Hair`` / ``HairShadow`` onto theirs. The ``Face`` is one mesh under one set of hashes
     on both, so it is left alone
     @endrst
     */
    ZZMIFixerConfig miyabiBlossomConfig();
}

#endif
