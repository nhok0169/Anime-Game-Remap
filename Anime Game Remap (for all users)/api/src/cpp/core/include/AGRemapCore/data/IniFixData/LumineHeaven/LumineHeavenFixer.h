#ifndef AGRemapCore_LumineHeavenFixer_H
#define AGRemapCore_LumineHeavenFixer_H

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
     LumineHeaven's own fixers :raw-html:`<br />` :raw-html:`<br />`

     The SEVENTH remap of a skin of SEVERAL components onto a target of ONE, and the inverse of
     :cpp:class:`LumineFixer`'s direction. See :cpp:func:`makeGIMIMergeFixer`. Pair this with
     :cpp:class:`LumineHeavenParser`. One parser option came out of it -- downloads that follow a slot's resource
     names (``GIMIComponentParserConfig::downloadsByName``)
     @endrst
     */
    class LumineHeavenFixer {
        public:

            LumineHeavenFixer() = delete;

            /**
             * @brief The fix onto Lumine -- what :cpp:func:`IniFixBuilderFuncs::lumineHeavenToLumine6_3` returns
             */
            static IniFixBuilder::Factory toLumine6_3();
    };
}

#endif
