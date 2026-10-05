#ifndef AGRemapCore_KaeyaParser_H
#define AGRemapCore_KaeyaParser_H

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
     Kaeya's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`KaeyaFixer`: the fixer's mod objects are the ones this parser
     classifies, and neither half makes sense alone
     @endrst
     */
    class KaeyaParser {
        public:

            KaeyaParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Kaeya ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::kaeya4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
