#ifndef AGRemapCore_AyakaFixer_H
#define AGRemapCore_AyakaFixer_H

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
     Ayaka's own ``.ini`` fixers, one per game version the remap needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`AyakaParser`. Ayaka and AyakaSpringbloom are a base/skin pair and
     remap onto each other
     @endrst
     */
    class AyakaFixer {
        public:

            AyakaFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for remapping an Ayaka mod onto AyakaSpringbloom -- what
             :cpp:func:`IniFixBuilderFuncs::ayaka6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
