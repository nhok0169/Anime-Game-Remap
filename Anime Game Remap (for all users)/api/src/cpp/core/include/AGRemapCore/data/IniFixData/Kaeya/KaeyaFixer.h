#ifndef AGRemapCore_KaeyaFixer_H
#define AGRemapCore_KaeyaFixer_H

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
     Kaeya's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`KaeyaParser`: the mod objects this fixer remaps are the ones
     that parser classifies, and neither half makes sense alone
     @endrst
     */
    class KaeyaFixer {
        public:

            KaeyaFixer() = delete;

            /**
             * @brief
             @rst
             The fix that remaps a Kaeya mod onto KaeyaSailwind at 6.1 -- what
             :cpp:func:`IniFixBuilderFuncs::kaeya6_1ToKaeyaSailwind` returns, and documented
             there
             @endrst
             */
            static IniFixBuilder::Factory v6_1ToKaeyaSailwind();
    };
}

#endif
