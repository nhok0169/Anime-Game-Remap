#ifndef AGRemapCore_XianglingFixer_H
#define AGRemapCore_XianglingFixer_H

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
     Xiangling's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`XianglingParser`, and read it next to :cpp:class:`XianglingCheerFixer`
     :raw-html:`<br />` :raw-html:`<br />`

     A MERGE, and the only fix in the repo still registered at **4.0** rather than 6.1 --
     deliberately. Her head re-issues ``ORFix``, and ORFix's maintainers baked the GI 6.1
     diffuse/lightmap register swap into the library itself, so a part that was already
     calling it needed no 6.1 row at all
     @endrst
     */
    class XianglingFixer {
        public:

            XianglingFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 4.0-era Xiangling ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::xiangling4_0` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v4_0();
    };
}

#endif
