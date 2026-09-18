#ifndef AGRemapCore_RaidenParser_H
#define AGRemapCore_RaidenParser_H

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
     Raiden's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Split out of :cpp:class:`IniParseBuilderData`'s own translation unit because a real generator
     is an order of magnitude larger than the stub it replaces -- a stub is one line, and
     \ref v6_1 is a parser subclass, a classifier configuration and the mapping tables behind it.
     :cpp:class:`IniParseBuilderFuncs` still declares the entry point (the table refers to it by
     that name); only the *definition* lives here
     @endrst
     */
    class RaidenParser {
        public:

            RaidenParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 6.1-era Raiden ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::raiden6_1` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v6_1();
    };
}

#endif
