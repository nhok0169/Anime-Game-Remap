#ifndef AGRemapCore_XingqiuParser_H
#define AGRemapCore_XingqiuParser_H

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
     Xingqiu's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`XingqiuFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     Xingqiu and XingqiuBamboo are a base/skin pair and remap onto each other. The skin
     draws a third object, so this direction is a split and the other a merge
     @endrst
     */
    class XingqiuParser {
        public:

            XingqiuParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Xingqiu ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::xingqiu4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
