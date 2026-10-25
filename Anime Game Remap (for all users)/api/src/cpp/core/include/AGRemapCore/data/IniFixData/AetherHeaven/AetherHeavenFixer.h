#ifndef AGRemapCore_AetherHeavenFixer_H
#define AGRemapCore_AetherHeavenFixer_H

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
     AetherHeaven's own fixers :raw-html:`<br />` :raw-html:`<br />`

     A skin of SEVERAL components merged onto a target of ONE, the inverse of :cpp:class:`AetherFixer`'s direction.
     See :cpp:func:`makeGIMIMergeFixer`. Pair this with :cpp:class:`AetherHeavenParser`
     @endrst
     */
    class AetherHeavenFixer {
        public:

            AetherHeavenFixer() = delete;

            /**
             * @brief The fix onto Aether -- what :cpp:func:`IniFixBuilderFuncs::aetherHeavenToAether6_3` returns
             */
            static IniFixBuilder::Factory toAether6_3();
    };
}

#endif
