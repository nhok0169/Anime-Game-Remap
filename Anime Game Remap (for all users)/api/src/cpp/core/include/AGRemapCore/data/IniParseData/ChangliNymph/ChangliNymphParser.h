#ifndef AGRemapCore_ChangliNymphParser_H
#define AGRemapCore_ChangliNymphParser_H

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
     ChangliNymph's (Changli's Laurel Nymph skin's) own ``.ini`` parsers, through
     :cpp:func:`makeWWMIParser`: eight draw slots on her ``vb0`` hash, the bone-data override and the two
     shape-key overrides by their own hashes. Pair this with :cpp:class:`ChangliNymphFixer`
     @endrst
     */
    class ChangliNymphParser {
        public:
            ChangliNymphParser() = delete;

            /**
             * @brief The 2.4 parser -- see :cpp:func:`IniParseBuilderFuncs::changliNymph2_4`
             */
            static IniParseBuilder::Factory v2_4();
    };
}

#endif
