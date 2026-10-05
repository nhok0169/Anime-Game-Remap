#ifndef AGRemapCore_ChisaParser_H
#define AGRemapCore_ChisaParser_H

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
     Chisa's own ``.ini`` parsers -- the second Wuthering Waves character, through
     :cpp:func:`makeWWMIParser`: seven draw slots on her ``vb0`` hash, the bone-data override and
     the two shape-key overrides by their own hashes. Pair this with :cpp:class:`ChisaFixer`

     .. note::
         She is past 256 merged bones, so a mod of hers carries WWMI's blend remap and its true
         16-bit ids live in ``BlendRemapVertexVG.buf`` rather than ``Blend.buf`` -- which matters to
         the FIXER, not here; the parse side is the same seven slots every WWMI character has
     @endrst
     */
    class ChisaParser {
        public:
            ChisaParser() = delete;

            /**
             * @brief The 2.8 parser -- see :cpp:func:`IniParseBuilderFuncs::chisa2_8`
             */
            static IniParseBuilder::Factory v2_8();
    };
}

#endif
