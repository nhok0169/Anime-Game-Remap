#ifndef AGRemapCore_CitlaliWhisperofStarsParser_H
#define AGRemapCore_CitlaliWhisperofStarsParser_H

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
     CitlaliWhisperofStars' own parsers :raw-html:`<br />` :raw-html:`<br />`

     A skin of THREE components -- a Body of four draw slots, a Bangs and an Eyes -- so it is built
     by :cpp:func:`makeGIMIComponentParser` rather than :cpp:func:`makeGIMICharParser`. Pair this
     with :cpp:class:`CitlaliWhisperofStarsFixer`
     @endrst
     */
    class CitlaliWhisperofStarsParser {
        public:

            CitlaliWhisperofStarsParser() = delete;

            /**
             * @brief Her 6.7 parser -- what :cpp:func:`IniParseBuilderFuncs::citlaliWhisperofStars6_7` returns
             */
            static IniParseBuilder::Factory v6_7();
    };
}

#endif
