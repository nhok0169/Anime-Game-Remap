#ifndef AGRemapCore_ChangliFixer_H
#define AGRemapCore_ChangliFixer_H

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
     Changli's own ``.ini`` fixers, through :cpp:func:`makeWWMIFixer`. Pair this with
     :cpp:class:`ChangliParser`

     What she needs that the earlier Wuthering Waves characters do not, each of it a field of
     :cpp:class:`WWMIFixerConfig` rather than anything of hers:

     * her blend is FOUR influences a vertex and her Laurel Nymph skin's draws read EIGHT, so the fix
       writes its blend at the target's width (:cpp:member:`WWMIFixerConfig::targetInfluences`)
     * her skeleton was renumbered twice (1.1 -> 1.2, 1.3 -> 1.4), so a mod is read with the vertex
       group row of the numbering its own geometry is in
       (:cpp:member:`WWMIFixerConfig::skeletonNumberings`)
     @endrst
     */
    class ChangliFixer {
        public:
            ChangliFixer() = delete;

            /**
             * @brief ``Changli -> ChangliNymph`` at game version ``2.4`` -- see
             *        :cpp:func:`IniFixBuilderFuncs::changliNymph2_4`
             */
            static IniFixBuilder::Factory nymph2_4();
    };
}

#endif
