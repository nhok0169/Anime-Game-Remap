#ifndef AGRemapCore_SanhuaParser_H
#define AGRemapCore_SanhuaParser_H

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
     Sanhua's own ``.ini`` parsers -- the first Wuthering Waves character, through
     :cpp:func:`makeWWMIParser`: seven draw slots on her ``vb0`` hash, the bone-data override and
     the two shape-key overrides by their own hashes. Pair this with :cpp:class:`SanhuaFixer`
     @endrst
     */
    class SanhuaParser {
        public:
            SanhuaParser() = delete;

            /**
             * @brief The 2.5 parser -- see :cpp:func:`IniParseBuilderFuncs::sanhua2_5`
             */
            static IniParseBuilder::Factory v2_5();
    };
}

#endif
