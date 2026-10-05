#ifndef AGRemapCore_HuTaoFixer_H
#define AGRemapCore_HuTaoFixer_H

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
     HuTao's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`HuTaoParser`, and read it next to :cpp:class:`CherryHuTaoFixer`
     :raw-html:`<br />` :raw-html:`<br />`

     A SPLIT of both her objects -- two become four -- plus a created normal map and a
     moved draw call
     @endrst
     */
    class HuTaoFixer {
        public:

            HuTaoFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era HuTao ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::hutao6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
