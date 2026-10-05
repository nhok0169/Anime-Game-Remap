#ifndef AGRemapCore_BennettFixer_H
#define AGRemapCore_BennettFixer_H

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
     Bennett's own ``.ini`` fixers -- the second remap onto a skin of SEVERAL COMPONENTS
     :raw-html:`<br />` :raw-html:`<br />`

     BennettAdventure (5.7) is a ``Body``, a ``Bang`` and an ``Eye``, so Bennett is fixed by THREE
     fixers, one per component, built from one :cpp:class:`GIMIComponentFixerConfig` -- see
     :cpp:func:`makeGIMIComponentFixer` for the shape and
     ``data/IniFixData/Bennett/BennettFixer.cpp`` for Bennett's own choices. Pair it with
     :cpp:class:`BennettParser`
     @endrst
     */
    class BennettFixer {
        public:

            BennettFixer() = delete;

            /**
             * @brief The fixer onto BennettAdventure's ``Body`` component -- what :cpp:func:`IniFixBuilderFuncs::bennettAdventureBody6_1` returns
             */
            static IniFixBuilder::Factory body6_1();

            /**
             * @brief The fixer onto BennettAdventure's ``Eye`` component -- what :cpp:func:`IniFixBuilderFuncs::bennettAdventureEye6_1` returns
             */
            static IniFixBuilder::Factory eye6_1();
    };
}

#endif
