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

#ifndef AGRemapCore_NeuvilletteMelusentFixer_H
#define AGRemapCore_NeuvilletteMelusentFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     NeuvilletteMelusent's own fixers :raw-html:`<br />` :raw-html:`<br />`

     The FIFTH remap of a skin of SEVERAL components onto a target of ONE, and the inverse of
     :cpp:class:`NeuvilletteFixer`'s direction. See :cpp:func:`makeGIMIMergeFixer`. Pair this with
     :cpp:class:`NeuvilletteMelusentParser`. Three template options came out of it: a component with a mod type
     name of its own (the unnamed main mesh), the target's texcoord stride, and a 16-bit mod's index counts
     @endrst
     */
    class NeuvilletteMelusentFixer {
        public:

            NeuvilletteMelusentFixer() = delete;

            /**
             * @brief The fix onto Neuvillette -- what :cpp:func:`IniFixBuilderFuncs::neuvilletteMelusentToNeuvillette6_3` returns
             */
            static IniFixBuilder::Factory toNeuvillette6_3();
    };
}

#endif
