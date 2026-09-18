#ifndef AGRemapCore_YelanTranquilFixer_H
#define AGRemapCore_YelanTranquilFixer_H

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
     YelanTranquil's own fixers :raw-html:`<br />` :raw-html:`<br />`

     The FIRST remap of a skin of SEVERAL components onto a target of ONE, and the inverse of
     :cpp:class:`YelanFixer`'s direction. See :cpp:func:`makeGIMIMergeFixer`. Pair this with
     :cpp:class:`YelanTranquilParser`
     @endrst
     */
    class YelanTranquilFixer {
        public:

            YelanTranquilFixer() = delete;

            /**
             * @brief The fix onto Yelan -- what :cpp:func:`IniFixBuilderFuncs::yelanTranquilToYelan6_1` returns
             */
            static IniFixBuilder::Factory toYelan6_1();
    };
}

#endif
