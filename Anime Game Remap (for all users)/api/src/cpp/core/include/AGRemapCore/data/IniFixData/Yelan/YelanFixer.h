#ifndef AGRemapCore_YelanFixer_H
#define AGRemapCore_YelanFixer_H

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
     Yelan's own ``.ini`` fixers -- the first remap onto a skin of SEVERAL COMPONENTS
     :raw-html:`<br />` :raw-html:`<br />`

     YelanTranquil (5.7) is a ``Body``, a ``Bang`` and an ``Eye``, so Yelan is fixed by THREE
     fixers, one per component, built from one :cpp:class:`GIMIComponentFixerConfig` -- see
     :cpp:func:`makeGIMIComponentFixer` for the shape and ``data/IniFixData/Yelan/YelanFixer.cpp``
     for Yelan's own choices (which slot each component draws through, the texture recipe, the
     band legend). Pair it with :cpp:class:`YelanParser`
     @endrst
     */
    class YelanFixer {
        public:

            YelanFixer() = delete;

            /**
             * @brief The fixer onto YelanTranquil's ``Body`` component -- what :cpp:func:`IniFixBuilderFuncs::yelanTranquilBody6_1` returns
             */
            static IniFixBuilder::Factory body6_1();

            /**
             * @brief The fixer onto YelanTranquil's ``Bang`` component -- what :cpp:func:`IniFixBuilderFuncs::yelanTranquilBang6_1` returns
             */
            static IniFixBuilder::Factory bang6_1();

            /**
             * @brief The fixer onto YelanTranquil's ``Eye`` component -- what :cpp:func:`IniFixBuilderFuncs::yelanTranquilEye6_1` returns
             */
            static IniFixBuilder::Factory eye6_1();
    };
}

#endif
