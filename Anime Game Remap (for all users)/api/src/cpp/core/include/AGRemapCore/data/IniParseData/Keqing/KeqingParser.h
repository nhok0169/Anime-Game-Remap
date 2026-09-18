#ifndef AGRemapCore_KeqingParser_H
#define AGRemapCore_KeqingParser_H

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
     Keqing's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`KeqingFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     Keqing and KeqingOpulent are a base/skin pair and remap onto each other -- and
     the pair is this repo's worked example of a MERGE and its inverse SPLIT, since
     KeqingOpulent has no ``dress`` of her own
     @endrst
     */
    class KeqingParser {
        public:

            KeqingParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Keqing ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::keqing4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
