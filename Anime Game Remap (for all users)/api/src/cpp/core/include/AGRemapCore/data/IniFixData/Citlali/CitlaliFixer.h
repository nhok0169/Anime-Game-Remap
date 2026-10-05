#ifndef AGRemapCore_CitlaliFixer_H
#define AGRemapCore_CitlaliFixer_H

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
     Citlali's own ``.ini`` fixers -- the third remap onto a skin of SEVERAL COMPONENTS
     :raw-html:`<br />` :raw-html:`<br />`

     CitlaliWhisperofStars (6.7) is a ``Body``, a ``Bangs`` and an ``Eyes``, so Citlali is fixed
     by THREE fixers, one per component, built from one :cpp:class:`GIMIComponentFixerConfig` --
     see :cpp:func:`makeGIMIComponentFixer` for the shape and
     ``data/IniFixData/Citlali/CitlaliFixer.cpp`` for Citlali's own choices. Pair it with
     :cpp:class:`CitlaliParser`
     @endrst
     */
    class CitlaliFixer {
        public:

            CitlaliFixer() = delete;

            /**
             * @brief The fixer onto CitlaliWhisperofStars's ``Body`` -- what :cpp:func:`IniFixBuilderFuncs::citlaliWhisperofStarsBody6_7` returns
             */
            static IniFixBuilder::Factory body6_7();

            /**
             * @brief The fixer onto CitlaliWhisperofStars's ``Bangs`` -- what :cpp:func:`IniFixBuilderFuncs::citlaliWhisperofStarsBangs6_7` returns
             */
            static IniFixBuilder::Factory bangs6_7();

            /**
             * @brief The fixer onto CitlaliWhisperofStars's ``Eyes`` -- what :cpp:func:`IniFixBuilderFuncs::citlaliWhisperofStarsEyes6_7` returns
             */
            static IniFixBuilder::Factory eyes6_7();
    };
}

#endif
