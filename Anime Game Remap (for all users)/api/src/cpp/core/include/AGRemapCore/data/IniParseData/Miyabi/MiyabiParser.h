#ifndef AGRemapCore_MiyabiParser_H
#define AGRemapCore_MiyabiParser_H

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
     Miyabi's own ``.ini`` parsers -- the first Zenless Zone Zero character, through
     :cpp:func:`makeZZMIParser` over :cpp:func:`miyabiBlossomConfig`. Pair this with
     :cpp:func:`makeZZMIFixer`'s rows for MiyabiBlossom
     @endrst
     */
    class MiyabiParser {
        public:
            MiyabiParser() = delete;

            /**
             * @brief The 3.2 parser -- see :cpp:func:`IniParseBuilderFuncs::miyabi3_2`
             */
            static IniParseBuilder::Factory v3_2();
    };
}

#endif
