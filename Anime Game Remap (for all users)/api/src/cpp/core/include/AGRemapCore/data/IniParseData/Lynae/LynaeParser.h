#ifndef AGRemapCore_LynaeParser_H
#define AGRemapCore_LynaeParser_H

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
     Lynae's own ``.ini`` parsers, through :cpp:func:`makeWWMIParser`: eight draw slots on her
     ``vb0`` hash, the bone-data override and the two shape-key overrides by their own hashes. Pair
     this with :cpp:class:`LynaeFixer`

     .. note::
         Filed at 3.6, the version whose ``vb0`` most of her mods carry; the library files her 3.7
         ``vb0`` too, and a section on either is hers
     @endrst
     */
    class LynaeParser {
        public:
            LynaeParser() = delete;

            /**
             * @brief The 3.6 parser -- see :cpp:func:`IniParseBuilderFuncs::lynae3_6`
             */
            static IniParseBuilder::Factory v3_6();
    };
}

#endif
