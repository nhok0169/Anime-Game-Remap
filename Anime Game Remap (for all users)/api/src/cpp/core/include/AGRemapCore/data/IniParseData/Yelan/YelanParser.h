#ifndef AGRemapCore_YelanParser_H
#define AGRemapCore_YelanParser_H

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
     Yelan's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     The standard GIMI character shape, drawing ``head`` / ``body`` / ``dress`` / ``extra``.
     Pair this with :cpp:class:`YelanFixer`
     @endrst
     */
    class YelanParser {
        public:

            YelanParser() = delete;

            /**
             * @brief The parser for a 4.0-era Yelan ``.ini`` file -- what :cpp:func:`IniParseBuilderFuncs::yelan4_0` returns
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
