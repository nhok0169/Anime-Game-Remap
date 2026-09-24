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

#ifndef AGRemapCore_CharlotteHurlockFixer_H
#define AGRemapCore_CharlotteHurlockFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     CharlotteHurlock's own fixers :raw-html:`<br />` :raw-html:`<br />`

     The FOURTH remap of a skin of SEVERAL components onto a target of ONE, and the inverse of
     :cpp:class:`CharlotteFixer`'s direction. See :cpp:func:`makeGIMIMergeFixer`. Pair this with
     :cpp:class:`CharlotteHurlockParser`. The same two template options CitlaliWhisperofStars
     needs -- ``targetLayout`` and ``texRegsByName`` -- and nothing of Charlotte's own beyond the
     config
     @endrst
     */
    class CharlotteHurlockFixer {
        public:

            CharlotteHurlockFixer() = delete;

            /**
             * @brief The fix onto Charlotte -- what :cpp:func:`IniFixBuilderFuncs::charlotteHurlockToCharlotte6_7` returns
             */
            static IniFixBuilder::Factory toCharlotte6_7();
    };
}

#endif
