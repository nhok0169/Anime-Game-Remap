#ifndef AGRemapCore_LynaePeppermintFixer_H
#define AGRemapCore_LynaePeppermintFixer_H

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
     LynaePeppermint's own ``.ini`` fixers -- the reverse of :cpp:class:`LynaeFixer`, through
     :cpp:func:`makeWWMIFixer`. What is specific to this direction:

     * **two source components share one target slot.** Lynae's props slot renders through a
       special-material (glass / foil) layer that ordinary cloth barely reaches, so the skin's coat and
       her shirt / jacket / shoes both go through Lynae's jacket slot, the second in a generated copy
       ``.ini``. On a target past 256 merged bones that copy has to share the mod's skeleton state
       (:cpp:member:`WWMIFixerConfig::copiesShareSkeleton`), and the slot's outline pass binds each
       source's own diffuse (:cpp:member:`WWMIFixerConfig::Binding::srcComponent`)
     * **Lynae draws passes with her skeleton in ``vs-cb3`` alone** (her early depth passes), which
       the merge list reads as the current pose (:cpp:member:`WWMIFixerConfig::currentPoseInCb3Only`)
     * **the skin's material masks mark the cloth under a sheer garment with R = 0**, which Lynae's
       shaders do not draw as cloth, so every body mask is repacked into her legend
       (:cpp:member:`WWMIFixerConfig::texEdits`)
     @endrst
     */
    class LynaePeppermintFixer {
        public:
            LynaePeppermintFixer() = delete;

            /**
             * @brief ``LynaePeppermint -> Lynae`` at game version ``3.7`` -- see
             *        :cpp:func:`IniFixBuilderFuncs::lynae3_7`
             */
            static IniFixBuilder::Factory lynae3_7();
    };
}

#endif
