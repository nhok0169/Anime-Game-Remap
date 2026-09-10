#ifndef AGRemapCore_HuTaoParser_H
#define AGRemapCore_HuTaoParser_H

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
     HuTao's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`HuTaoFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     HuTao and CherryHuTao are a base/skin pair and remap onto each other. The skin
     draws FOUR objects to HuTao's two, so this direction is a split and the other a
     merge
     @endrst
     */
    class HuTaoParser {
        public:

            HuTaoParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era HuTao ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::hutao4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
