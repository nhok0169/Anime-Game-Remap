#ifndef AGRemapCore_NeuvilletteFixer_H
#define AGRemapCore_NeuvilletteFixer_H

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
     Neuvillette's own ``.ini`` fixers -- onto NeuvilletteMelusent, a skin of FOUR components whose main
     mesh is unnamed :raw-html:`<br />` :raw-html:`<br />`

     NeuvilletteMelusent (6.3) is a main mesh (component ``""``, three draw slots), a ``Coat``, a ``Bang``
     and an ``Eye``, so Neuvillette is fixed by FOUR fixers, one per component, built from one
     :cpp:class:`GIMIComponentFixerConfig` -- see :cpp:func:`makeGIMIComponentFixer` for the shape and
     ``data/IniFixData/Neuvillette/NeuvilletteFixer.cpp`` for his own choices. Pair it with
     :cpp:class:`NeuvilletteParser`
     @endrst
     */
    class NeuvilletteFixer {
        public:

            NeuvilletteFixer() = delete;

            /**
             * @brief The fixer onto NeuvilletteMelusent's main mesh -- what :cpp:func:`IniFixBuilderFuncs::neuvilletteMelusentMain6_3` returns
             */
            static IniFixBuilder::Factory main6_3();

            /**
             * @brief The fixer onto NeuvilletteMelusent's ``Coat`` -- what :cpp:func:`IniFixBuilderFuncs::neuvilletteMelusentCoat6_3` returns
             */
            static IniFixBuilder::Factory coat6_3();

            /**
             * @brief The fixer onto NeuvilletteMelusent's ``Bang`` -- what :cpp:func:`IniFixBuilderFuncs::neuvilletteMelusentBang6_3` returns
             */
            static IniFixBuilder::Factory bang6_3();

            /**
             * @brief The fixer onto NeuvilletteMelusent's ``Eye`` -- what :cpp:func:`IniFixBuilderFuncs::neuvilletteMelusentEye6_3` returns
             */
            static IniFixBuilder::Factory eye6_3();
    };
}

#endif
