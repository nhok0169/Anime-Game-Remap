#ifndef AGRemapCore_NilouFixer_H
#define AGRemapCore_NilouFixer_H

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
     Nilou's own ``.ini`` fixers, one per game version the remap needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`NilouParser`. Nilou and NilouBreeze are a base/skin pair and remap onto each other
     @endrst
     */
    class NilouFixer {
        public:

            NilouFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for remapping a Nilou mod onto NilouBreeze -- what
             :cpp:func:`IniFixBuilderFuncs::nilou5_7` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v5_7();
    };
}

#endif
