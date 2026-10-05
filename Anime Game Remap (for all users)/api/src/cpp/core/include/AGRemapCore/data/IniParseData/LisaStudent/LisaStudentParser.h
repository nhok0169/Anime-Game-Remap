#ifndef AGRemapCore_LisaStudentParser_H
#define AGRemapCore_LisaStudentParser_H

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
     LisaStudent's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`LisaStudentFixer`: the fixer's mod objects are the ones this parser
     classifies, and neither half makes sense alone
     @endrst
     */
    class LisaStudentParser {
        public:

            LisaStudentParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era LisaStudent ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::lisaStudent4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();

            /**
             * @brief
             @rst
             The parser for a 5.4-era LisaStudent ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::lisaStudent5_4` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v5_4();

            /**
             * @brief
             @rst
             The parser for a 5.7-era LisaStudent ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::lisaStudent5_7` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v5_7();
    };
}

#endif
