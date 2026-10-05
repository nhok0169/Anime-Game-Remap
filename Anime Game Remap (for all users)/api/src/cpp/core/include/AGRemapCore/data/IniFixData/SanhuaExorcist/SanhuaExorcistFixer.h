#ifndef AGRemapCore_SanhuaExorcistFixer_H
#define AGRemapCore_SanhuaExorcistFixer_H

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
     SanhuaExorcist's own ``.ini`` fixers -- her skin's mods onto Sanhua herself, the REVERSE of
     :cpp:class:`SanhuaFixer`, through :cpp:func:`makeWWMIFixer`. Pair this with
     :cpp:class:`SanhuaExorcistParser`
     @endrst
     */
    class SanhuaExorcistFixer {
        public:
            SanhuaExorcistFixer() = delete;

            /**
             * @brief The 2.5 fix remapping **SanhuaExorcist onto Sanhua** -- see :cpp:func:`IniFixBuilderFuncs::sanhua2_5`
             */
            static IniFixBuilder::Factory sanhua2_5();
    };
}

#endif
