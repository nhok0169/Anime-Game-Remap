#ifndef AGRemapCore_ZZMIBuilder_H
#define AGRemapCore_ZZMIBuilder_H

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

#include "AGRemapCore/model/strategies/ModType.h"
#include <vector>


namespace AGRemapCore {
    /**
     * @brief
     @rst
     Creates new :cpp:class:`ModType` objects for ZZZ (Zenless Zone Zero) mods -- the ZZMI
     counterpart of :cpp:class:`GIBuilder` :raw-html:`<br />` :raw-html:`<br />`

     A ZZMI mod is GIMI-shaped (per-component ``Position`` / ``Texcoord`` / ``Blend`` buffers, objects by
     ``match_first_index``), so a ZZZ mod type carries the same asset tables a GI one does and reads the
     same builder data tables. Every component of a ZZZ character skins from its own bone palette, so a
     character is also a remap source under each of its component names
     (:cpp:func:`ModTypeIdTools::getComponentIds`)
     @endrst
     */
    class ZZMIBuilder {
        public:
            /**
             * @brief Creates the :cpp:class:`ModType` for Miyabi
             */
            static ModType miyabi();

            /**
             * @brief Creates the :cpp:class:`ModType` for MiyabiBlossom (Miyabi's Dignified Blossom skin)
             */
            static ModType miyabiBlossom();

            /**
             * @brief Every ZZZ :cpp:class:`ModType`, freshly built on each call
             */
            static std::vector<ModType> all();
    };
}

#endif
