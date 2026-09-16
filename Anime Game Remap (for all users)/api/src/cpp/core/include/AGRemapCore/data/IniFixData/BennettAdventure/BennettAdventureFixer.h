#ifndef AGRemapCore_BennettAdventureFixer_H
#define AGRemapCore_BennettAdventureFixer_H

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
     BennettAdventure's own ``.ini`` fixer -- a skin of SEVERAL components merged onto a target of
     one :raw-html:`<br />` :raw-html:`<br />`

     The inverse of :cpp:class:`BennettFixer`, and the second character pair to need
     :cpp:func:`makeGIMIMergeFixer` after YelanTranquil. Always ONE ``.ini`` group out. Pair it with
     :cpp:class:`BennettAdventureParser`
     @endrst
     */
    class BennettAdventureFixer {
        public:

            BennettAdventureFixer() = delete;

            /**
             * @brief The fixer remapping BennettAdventure onto Bennett -- what :cpp:func:`IniFixBuilderFuncs::bennettAdventureToBennett6_1` returns
             */
            static IniFixBuilder::Factory toBennett6_1();
    };
}

#endif
