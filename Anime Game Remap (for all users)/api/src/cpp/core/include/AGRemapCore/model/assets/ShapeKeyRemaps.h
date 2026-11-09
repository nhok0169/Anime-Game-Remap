#ifndef AGRemapCore_ShapeKeyRemaps_H
#define AGRemapCore_ShapeKeyRemaps_H
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

#include <string>

#include "AGRemapCore/model/assets/ModAssets.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     The order a mod's shape keys are put in when it is remapped onto another character -- a :cpp:class:`ModAssets`
     pre-populated with this project's real data (:cpp:func:`Data::getShapeKeyRemapDataRows`) :raw-html:`<br />`
     :raw-html:`<br />`

     The game drives a character's shape keys (blinks, mouth shapes) by SLOT, and two characters can number the
     same keys differently, so a fix writes the mod's shape-key buffers again in the target's order
     (:cpp:func:`WWMIShapeKeys::reorder`). The value is that order: for each of the target's slots, the source key
     it takes (``-1`` for none), comma-separated :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Six index columns with two versions, as :cpp:class:`VGRemaps` has, for the same reason -- an order is
        defined between a version of one character and a version of another: ``fromVersion`` (version),
        ``fromChar``, ``fromComp``, ``toVersion`` (version), ``toChar``, ``toComp``. Both component columns are
        ``""`` on every shipped row. See :cpp:func:`ModType::getShapeKeyRemap`
     @endrst
     */
    class ShapeKeyRemaps: public ModAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated table
             */
            ShapeKeyRemaps();
    };
}

#endif
