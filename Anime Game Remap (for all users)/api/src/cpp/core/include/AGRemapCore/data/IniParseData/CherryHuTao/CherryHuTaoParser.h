#ifndef AGRemapCore_CherryHuTaoParser_H
#define AGRemapCore_CherryHuTaoParser_H

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
     CherryHuTao's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`CherryHuTaoFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     FOUR drawn objects, two more than HuTao. The fixer is the most involved in the
     repo: three texture edits, reflection sections to strip, a register shift and a
     TexFx re-issue
     @endrst
     */
    class CherryHuTaoParser {
        public:

            CherryHuTaoParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 5.3-era CherryHuTao ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::cherryHutao5_3` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v5_3();
    };
}

#endif
