#ifndef AGRemapCore_XianglingCheerParser_H
#define AGRemapCore_XianglingCheerParser_H

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

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     XianglingCheer's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`XianglingCheerFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     Draws NO ``dress`` -- see :cpp:class:`XianglingCheerFixer` for what Xiangling's
     third object becomes on the way over
     @endrst
     */
    class XianglingCheerParser {
        public:

            XianglingCheerParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 5.3-era XianglingCheer ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::xianglingCheer5_3` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v5_3();
    };
}

#endif
