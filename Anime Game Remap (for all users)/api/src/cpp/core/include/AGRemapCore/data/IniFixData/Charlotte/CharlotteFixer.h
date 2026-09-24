#ifndef AGRemapCore_CharlotteFixer_H
#define AGRemapCore_CharlotteFixer_H

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
     Charlotte's own ``.ini`` fixers -- the fourth remap onto a skin of SEVERAL COMPONENTS
     :raw-html:`<br />` :raw-html:`<br />`

     CharlotteHurlock (6.7) is a ``Body``, a ``Bangs``, an ``Eyes`` and a ``Camera``. Charlotte is
     fixed by TWO fixers, onto the ``Body`` and the ``Eyes``, built from one
     :cpp:class:`GIMIComponentFixerConfig` -- see :cpp:func:`makeGIMIComponentFixer` for the shape
     and ``data/IniFixData/Charlotte/CharlotteFixer.cpp`` for Charlotte's own choices. Pair it with
     :cpp:class:`CharlotteParser`
     @endrst
     */
    class CharlotteFixer {
        public:

            CharlotteFixer() = delete;

            /**
             * @brief The fixer onto CharlotteHurlock's ``Body`` -- what :cpp:func:`IniFixBuilderFuncs::charlotteHurlockBody6_7` returns
             */
            static IniFixBuilder::Factory body6_7();

            /**
             * @brief The fixer onto CharlotteHurlock's ``Eyes`` -- what :cpp:func:`IniFixBuilderFuncs::charlotteHurlockEyes6_7` returns
             */
            static IniFixBuilder::Factory eyes6_7();
    };
}

#endif
