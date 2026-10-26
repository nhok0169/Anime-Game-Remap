#ifndef AGRemapCore_ChangliParser_H
#define AGRemapCore_ChangliParser_H

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
     Changli's own ``.ini`` parsers, through :cpp:func:`makeWWMIParser`: eight draw slots on her
     ``vb0`` hash, the bone-data override and the two shape-key overrides by their own hashes. Pair
     this with :cpp:class:`ChangliFixer`

     .. note::
         Filed at 1.1, her oldest mesh: the library files her 1.2 and 1.4 hashes too, so a mod of any
         of her three mesh generations is read
     @endrst
     */
    class ChangliParser {
        public:
            ChangliParser() = delete;

            /**
             * @brief The 1.1 parser -- see :cpp:func:`IniParseBuilderFuncs::changli1_1`
             */
            static IniParseBuilder::Factory v1_1();
    };
}

#endif
