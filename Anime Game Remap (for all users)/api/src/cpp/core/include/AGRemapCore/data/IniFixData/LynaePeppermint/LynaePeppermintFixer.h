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

#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     LynaePeppermint's own ``.ini`` fixers -- the reverse of :cpp:class:`LynaeFixer`, through
     :cpp:func:`makeWWMIFixer`. What is specific to this direction:

     * **two source components share one target slot.** Lynae's props slot renders through a
       special-material (glass / foil) layer that ordinary cloth barely reaches, so the skin's coat and
       her shirt / jacket / shoes both go through Lynae's lower-body slot, drawn as further sections of the
       mod's own ``.ini`` (:cpp:member:`WWMIFixerConfig::SourceComponent::sameFileSection`: a copy ``.ini`` never
       fills the shape-keyed positions), and the slot's outline pass binds each source's own diffuse
       (:cpp:member:`WWMIFixerConfig::Binding::srcComponent`)
     * **Lynae draws passes with her skeleton in ``vs-cb3`` alone** (her early depth passes), which
       the merge list reads as the current pose (:cpp:member:`WWMIFixerConfig::currentPoseInCb3Only`)
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

            /**
             * @brief The config #lynae3_7 builds its fixer from (:cpp:func:`makeWWMIFixer`) -- to start a variant of the
             * shipped fix from, or to read what it does
             */
            static WWMIFixerConfig lynae3_7Config();
    };
}

#endif
