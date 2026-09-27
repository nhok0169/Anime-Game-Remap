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

#ifndef AGRemapCore_CharlotteHurlockParser_H
#define AGRemapCore_CharlotteHurlockParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     CharlotteHurlock's own parsers :raw-html:`<br />` :raw-html:`<br />`

     A skin of FOUR components (Body, Bangs, Eyes, Camera), read one component at a time -- see
     :cpp:func:`makeGIMIComponentParser`. The Camera is an accessory rather than part of her, so it
     is not configured and nothing of it is read. Pair this with :cpp:class:`CharlotteHurlockFixer`
     @endrst
     */
    class CharlotteHurlockParser {
        public:

            CharlotteHurlockParser() = delete;

            /**
             * @brief The parser for a 6.7 CharlotteHurlock mod -- what :cpp:func:`IniParseBuilderFuncs::charlotteHurlock6_7` returns
             */
            static IniParseBuilder::Factory v6_7();
    };
}

#endif
