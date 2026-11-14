#ifndef AGRemapCore_ShapeKeyDispatchSizes_H
#define AGRemapCore_ShapeKeyDispatchSizes_H
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
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/assets/ModMappedAssets.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     The height WWMI dispatches a character's shape keys with, per game version -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getShapeKeyDispatchSizeDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     Four index columns -- ``version`` (at position 0), ``name``, ``component`` and ``type`` -- as
     :cpp:class:`Indices` has, so :cpp:class:`ModMappedAssets`'s query methods take three non-version values
     positionally, in that order. A lookup at a version answers with the newest row at or below it
     :raw-html:`<br />` :raw-html:`<br />`

     The ``component`` is ``""`` and the ``type`` is ``dispatchY``: its value is the character's
     ``Metadata.json`` ``dispatch_y``, which a mod remapped onto the character writes as its
     ``$\\WWMIv1\\shapekey_dispatch_size_y_original_batch<n>``
     @endrst
     */
    class ShapeKeyDispatchSizes: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit ShapeKeyDispatchSizes(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
