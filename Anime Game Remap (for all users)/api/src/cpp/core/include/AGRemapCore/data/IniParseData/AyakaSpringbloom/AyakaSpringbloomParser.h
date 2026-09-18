#ifndef AGRemapCore_AyakaSpringbloomParser_H
#define AGRemapCore_AyakaSpringbloomParser_H

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
     AyakaSpringbloom's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`AyakaSpringbloomFixer`; the two agree on the mod object names by
     hand, and neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     AyakaSpringbloom is Ayaka's Springbloom Missive skin, and the two remap onto each other
     @endrst
     */
    class AyakaSpringbloomParser {
        public:

            AyakaSpringbloomParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era AyakaSpringbloom ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::ayakaSpringbloom4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();

            /**
             * @brief
             @rst
             The parser for a 5.6-era AyakaSpringbloom ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::ayakaSpringbloom5_6` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v5_6();

            /**
             * @brief
             @rst
             The parser for a 5.7-era AyakaSpringbloom ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::ayakaSpringbloom5_7` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v5_7();
    };
}

#endif
