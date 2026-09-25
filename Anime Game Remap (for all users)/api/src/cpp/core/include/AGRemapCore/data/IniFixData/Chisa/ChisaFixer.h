#ifndef AGRemapCore_ChisaFixer_H
#define AGRemapCore_ChisaFixer_H

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
     Chisa's own ``.ini`` fixers -- the second Wuthering Waves pair, through
     :cpp:func:`makeWWMIFixer`. Pair this with :cpp:class:`ChisaParser`

     What she needs that Sanhua does not, each of it a field of
     :cpp:class:`WWMIFixerConfig` rather than anything of hers:

     * every pass gated through its VERTEX shaders
       (:cpp:member:`WWMIFixerConfig::passVertexShaders`), because tagging a pixel shader switches
       RabbitFX off for every mod drawing with it
     * a slot's other passes binding the same art at different registers
       (:cpp:member:`WWMIFixerConfig::extraPassRegs`)
     * the second mesh she draws, her hair ribbon
       (:cpp:member:`WWMIFixerConfig::sharedMeshes`)
     * the three lines a mod of a character past 256 merged bones carries, which are an exact
       inverse of the fix (:cpp:member:`WWMIFixerConfig::removedRegs`)
     * two parts the skin has no counterpart for, pinned to one bone each
       (:cpp:member:`WWMIFixerConfig::anchorChains`)
     * three texture edits (:cpp:member:`WWMIFixerConfig::texEdits`): her material mask repacked
       into the target's layout, her packed sheen matcap translated into the skin's foil, and a
       colour grade on the UV island her hair ribbon occupies
     @endrst
     */
    class ChisaFixer {
        public:
            ChisaFixer() = delete;

            /**
             * @brief ``Chisa -> ChisaParfait`` at game version ``3.5`` -- see
             *        :cpp:func:`IniFixBuilderFuncs::chisaParfait3_5`
             */
            static IniFixBuilder::Factory parfait3_5();
    };
}

#endif
