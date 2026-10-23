#ifndef AGRemapCore_ChangliNymphFixer_H
#define AGRemapCore_ChangliNymphFixer_H

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

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     ChangliNymph's (Changli's Laurel Nymph skin's) own ``.ini`` fixers, through :cpp:func:`makeWWMIFixer`.
     Pair this with :cpp:class:`ChangliNymphParser`

     Her blend is EIGHT influences a vertex and Changli's draws read four, so the fix keeps each vertex's
     four heaviest (:cpp:member:`WWMIFixerConfig::targetInfluences`)
     @endrst
     */
    class ChangliNymphFixer {
        public:
            ChangliNymphFixer() = delete;

            /**
             * @brief ``ChangliNymph -> Changli`` at game version ``1.4`` -- see
             *        :cpp:func:`IniFixBuilderFuncs::changli1_4`
             */
            static IniFixBuilder::Factory changli1_4();
    };
}

#endif
