#ifndef AGRemapCore_KeqingOpulentParser_H
#define AGRemapCore_KeqingOpulentParser_H

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
     KeqingOpulent's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`KeqingOpulentFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     KeqingOpulent draws NO ``dress`` -- her outfit is one body mesh, which is exactly
     what makes remapping to and from Keqing a merge in one direction and a split in
     the other
     @endrst
     */
    class KeqingOpulentParser {
        public:

            KeqingOpulentParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era KeqingOpulent ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::keqingOpulent4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
