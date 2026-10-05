#ifndef AGRemapCore_KiraraBootsParser_H
#define AGRemapCore_KiraraBootsParser_H

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
     KiraraBoots' own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`KiraraBootsFixer`; the two agree on the mod object names by hand,
     and neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     KiraraBoots is Kirara's summer skin, and the two remap onto each other
     @endrst
     */
    class KiraraBootsParser {
        public:

            KiraraBootsParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.8-era KiraraBoots ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::kiraraBoots4_8` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_8();

            /**
             * @brief
             @rst
             The parser for a 5.7-era KiraraBoots ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::kiraraBoots5_7` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v5_7();
    };
}

#endif
