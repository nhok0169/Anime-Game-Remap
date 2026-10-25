#ifndef AGRemapCore_AetherParser_H
#define AGRemapCore_AetherParser_H

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
     Aether's own ``.ini`` parsers :raw-html:`<br />` :raw-html:`<br />`

     The standard GIMI character shape, drawing ``head`` / ``body`` -- both on the plain texture layout.
     Pair this with :cpp:class:`AetherFixer`
     @endrst
     */
    class AetherParser {
        public:

            AetherParser() = delete;

            /**
             * @brief The parser for a 4.0-era Aether ``.ini`` file -- what :cpp:func:`IniParseBuilderFuncs::aether4_0` returns
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
