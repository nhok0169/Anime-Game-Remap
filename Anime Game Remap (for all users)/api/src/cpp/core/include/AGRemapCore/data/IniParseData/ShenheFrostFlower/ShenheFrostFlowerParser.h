#ifndef AGRemapCore_ShenheFrostFlowerParser_H
#define AGRemapCore_ShenheFrostFlowerParser_H

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
     ShenheFrostFlower's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`ShenheFrostFlowerFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     FOUR drawn objects, one more than Shenhe -- see :cpp:class:`ShenheFrostFlowerFixer`
     for what the extra one becomes on the way back
     @endrst
     */
    class ShenheFrostFlowerParser {
        public:

            ShenheFrostFlowerParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.4-era ShenheFrostFlower ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::shenheFrostFlower4_4` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_4();
    };
}

#endif
