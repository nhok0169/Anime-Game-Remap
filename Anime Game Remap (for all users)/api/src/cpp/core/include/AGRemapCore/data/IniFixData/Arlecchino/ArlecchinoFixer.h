#ifndef AGRemapCore_ArlecchinoFixer_H
#define AGRemapCore_ArlecchinoFixer_H

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
     Arlecchino's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`ArlecchinoParser`: the mod objects this fixer remaps are the
     ones that parser classifies, and neither half makes sense alone
     @endrst
     */
    class ArlecchinoFixer {
        public:

            ArlecchinoFixer() = delete;

            /**
             * @brief
             @rst
             The fix that remaps an Arlecchino mod onto ArlecchinoBoss at 5.7 -- what
             :cpp:func:`IniFixBuilderFuncs::arlecchino5_7` returns, and documented there
             :raw-html:`<br />` :raw-html:`<br />`

             There is no 6.1 row for her in the pure-Python table, so this one serves 6.1 too --
             the same arrangement :cpp:func:`IniFixBuilderFuncs::nilou5_7` has
             @endrst
             */
            static IniFixBuilder::Factory v5_7();
    };
}

#endif
