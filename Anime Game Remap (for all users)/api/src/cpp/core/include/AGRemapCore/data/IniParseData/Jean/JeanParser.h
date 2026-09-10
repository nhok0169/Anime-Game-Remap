#ifndef AGRemapCore_JeanParser_H
#define AGRemapCore_JeanParser_H

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
     Jean's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`JeanFixer`. Jean is the first character whose fix is **two** fixers
     rather than one -- she remaps onto JeanCN *and* onto JeanSea, and those are different shapes --
     but the parser is shared between them, because what a Jean ``.ini`` file *contains* does not
     depend on what it is being remapped to
     @endrst
     */
    class JeanParser {
        public:

            JeanParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Jean ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::jean4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();

            /**
             * @brief
             @rst
             The parser for a 5.5-era Jean ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::jean5_5` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v5_5();
    };
}

#endif
