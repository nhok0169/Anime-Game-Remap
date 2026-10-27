#ifndef AGRemapCore_AetherHeavenParser_H
#define AGRemapCore_AetherHeavenParser_H

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
     AetherHeaven's own ``.ini`` parsers :raw-html:`<br />` :raw-html:`<br />`

     A skin of three components -- an unnamed main mesh, a Bang and an Eye. See :cpp:func:`makeGIMIComponentParser`.
     Pair this with :cpp:class:`AetherHeavenFixer`
     @endrst
     */
    class AetherHeavenParser {
        public:

            AetherHeavenParser() = delete;

            /**
             * @brief The parser for a 6.3-era AetherHeaven ``.ini`` file -- what :cpp:func:`IniParseBuilderFuncs::aetherHeaven6_3` returns
             */
            static IniParseBuilder::Factory v6_3();
    };
}

#endif
