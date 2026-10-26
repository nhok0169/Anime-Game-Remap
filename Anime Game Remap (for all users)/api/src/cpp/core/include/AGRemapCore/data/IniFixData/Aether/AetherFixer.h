#ifndef AGRemapCore_AetherFixer_H
#define AGRemapCore_AetherFixer_H

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
     Aether's own ``.ini`` fixers -- onto AetherHeaven, a skin of THREE components whose main mesh is unnamed
     :raw-html:`<br />` :raw-html:`<br />`

     AetherHeaven (6.3) is a main mesh (component ``""``, two draw slots), a ``Bang`` and an ``Eye``, so Aether is
     fixed by THREE fixers, one per component, built from one :cpp:class:`GIMIComponentFixerConfig` -- see
     :cpp:func:`makeGIMIComponentFixer` for the shape and ``data/IniFixData/Aether/AetherFixer.cpp`` for his own
     choices. Pair it with :cpp:class:`AetherParser`
     @endrst
     */
    class AetherFixer {
        public:

            AetherFixer() = delete;

            /**
             * @brief The fixer onto AetherHeaven's main mesh -- what :cpp:func:`IniFixBuilderFuncs::aetherHeavenMain6_3` returns
             */
            static IniFixBuilder::Factory main6_3();

            /**
             * @brief The fixer onto AetherHeaven's ``Bang`` -- what :cpp:func:`IniFixBuilderFuncs::aetherHeavenBang6_3` returns
             */
            static IniFixBuilder::Factory bang6_3();

            /**
             * @brief The fixer onto AetherHeaven's ``Eye`` -- what :cpp:func:`IniFixBuilderFuncs::aetherHeavenEye6_3` returns
             */
            static IniFixBuilder::Factory eye6_3();
    };
}

#endif
