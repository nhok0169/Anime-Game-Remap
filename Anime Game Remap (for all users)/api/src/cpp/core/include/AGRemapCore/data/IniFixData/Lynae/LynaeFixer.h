#ifndef AGRemapCore_LynaeFixer_H
#define AGRemapCore_LynaeFixer_H

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
     Lynae's own ``.ini`` fixers, through :cpp:func:`makeWWMIFixer`. Pair this with
     :cpp:class:`LynaeParser`

     What she needs that the earlier Wuthering Waves characters do not, each of it a field of
     :cpp:class:`WWMIFixerConfig` rather than anything of hers:

     * her skeleton was RENUMBERED by a game update, so a mod is read with the vertex group row of the
       numbering its own geometry says it is in (:cpp:member:`WWMIFixerConfig::skeletonNumberings`,
       :cpp:member:`WWMIFixerConfig::referenceBoneCentroids`) -- its ``vb0`` cannot say, since
       hash-update tools rewrite the hash and keep the bone ids
     * a newer export's batched shape keys need the target's dispatch height
       (:cpp:member:`WWMIFixerConfig::shapeKeyDispatchSize`)
     * a mod from before WWMI's merged skeleton is lifted into her 3.6 numbering
       (:cpp:member:`WWMIFixerConfig::sourceVgMaps`), which is the only numbering such a mod of hers can
       be in
     @endrst
     */
    class LynaeFixer {
        public:
            LynaeFixer() = delete;

            /**
             * @brief ``Lynae -> LynaePeppermint`` at game version ``3.7`` -- see
             *        :cpp:func:`IniFixBuilderFuncs::lynaePeppermint3_7`
             */
            static IniFixBuilder::Factory peppermint3_7();
    };
}

#endif
