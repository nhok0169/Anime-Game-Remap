#ifndef AGRemapCore_ChisaParfaitParser_H
#define AGRemapCore_ChisaParfaitParser_H

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
     ChisaParfait's own ``.ini`` parsers -- through :cpp:func:`makeWWMIParser`: EIGHT draw slots on
     her ``vb0`` hash (one more than Chisa, her component 7 being a small right-hip prop), the
     bone-data override and the two shape-key overrides by their own hashes. Pair this with
     :cpp:class:`ChisaParfaitFixer`

     .. note::
         Unlike Chisa she does NOT carry WWMI's blend remap: her merged skeleton is 264 slots and the
         bones her own model actually weights stop at 250, so her ``Blend.buf`` holds plain 8-bit
         merged ids. Her mods are 8-influence all the same (16 bytes a vertex), which is what
         ``wwmiBlendInfluences`` in the fixer is for
     @endrst
     */
    class ChisaParfaitParser {
        public:
            ChisaParfaitParser() = delete;

            /**
             * @brief The 3.5 parser -- see :cpp:func:`IniParseBuilderFuncs::chisaParfait3_5`
             */
            static IniParseBuilder::Factory v3_5();
    };
}

#endif
