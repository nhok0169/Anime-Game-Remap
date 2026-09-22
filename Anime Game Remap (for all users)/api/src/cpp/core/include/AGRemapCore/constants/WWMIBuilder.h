#ifndef AGRemapCore_WWMIBuilder_H
#define AGRemapCore_WWMIBuilder_H

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
     Creates new :cpp:class:`ModType` objects for WuWa (Wuthering Waves) mods -- the WWMI
     counterpart of :cpp:class:`GIBuilder`, added 2026-09-19 with Sanhua and her Exorcist skin as
     its first pair :raw-html:`<br />` :raw-html:`<br />`

     A WWMI mod type carries the same four asset tables a GI one does plus the four WWMI-only ones
     (:cpp:class:`IndexCounts`, :cpp:class:`VGOffsets`, :cpp:class:`VGCounts`,
     :cpp:class:`ShapeKeyChecksums`), all keyed by the same remap graph
     (:cpp:func:`ModTypeIdTools::getHashRemapTargets`). Its parse / fix / remove rows are STUBS
     (``defaultFactory``) until the WWMI strategies exist; the prototype of the fix is
     ``Tools/Misc/Prototypes/sanhuaExorcistFix.py``
     @endrst
     */
    class WWMIBuilder {
        public:
            /**
             * @brief Creates the :cpp:class:`ModType` for Sanhua
             */
            static ModType sanhua();

            /**
             * @brief Creates the :cpp:class:`ModType` for SanhuaExorcist (WWMI-Assets' ``SanhuaSkin1``)
             */
            static ModType sanhuaExorcist();

            /**
             * @brief Creates the :cpp:class:`ModType` for Chisa
             */
            static ModType chisa();

            /**
             * @brief Creates the :cpp:class:`ModType` for ChisaParfait
             */
            static ModType chisaParfait();

            /**
             * @brief Every WuWa :cpp:class:`ModType`, freshly built on each call
             */
            static std::vector<ModType> all();
    };
}

#endif
