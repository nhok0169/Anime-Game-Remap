#ifndef AGRemapCore_XingqiuBambooParser_H
#define AGRemapCore_XingqiuBambooParser_H

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
     XingqiuBamboo's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`XingqiuBambooFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     Draws a ``dress`` Xingqiu has no geometry for, so remapping back onto him is a
     MERGE -- see :cpp:class:`XingqiuBambooFixer`
     @endrst
     */
    class XingqiuBambooParser {
        public:

            XingqiuBambooParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.4-era XingqiuBamboo ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::xingqiuBamboo4_4` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_4();
    };
}

#endif
