#ifndef AGRemapCore_SanhuaFixer_H
#define AGRemapCore_SanhuaFixer_H

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

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     Sanhua's own ``.ini`` fixers -- the first Wuthering Waves remap, Sanhua onto her Exorcist
     skin, through :cpp:func:`makeWWMIFixer`. Pair this with :cpp:class:`SanhuaParser`
     @endrst
     */
    class SanhuaFixer {
        public:
            SanhuaFixer() = delete;

            /**
             * @brief The 2.5 fix remapping **Sanhua onto SanhuaExorcist** -- see :cpp:func:`IniFixBuilderFuncs::sanhuaExorcist2_5`
             */
            static IniFixBuilder::Factory exorcist2_5();
    };
}

#endif
