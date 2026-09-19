#ifndef AGRemapCore_IndexCounts_H
#define AGRemapCore_IndexCounts_H

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
     The ``match_index_count`` of a wwmi draw slot -- how many indices the game's draw of that slot covers, which a mod's ``[textureoverridecomponentn]`` matches on together with the slot's ``match_first_index`` (an :cpp:class:`indices` row) -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getIndexCountDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     The exact sibling of :cpp:class:`Indices`, added for Wuthering Waves (2026-09-19): read that
     class first, everything there applies here too. Same **4** index columns -- ``version`` (the
     version index, at position 0), ``name``, ``component`` and ``type`` -- so
     :cpp:class:`ModMappedAssets`'s query methods take three non-version values positionally, in
     that order; ``component`` is ``""`` on every row (a WWMI character's several draw slots are
     ONE merged skeleton, so the slot goes in ``type``, as ``component0``, ``component1``, ...),
     a real key value rather than a "missing" marker :raw-html:`<br />` :raw-html:`<br />`

     A GIMI character has no rows here: this is the shape a WWMI ``.ini`` carries beside the
     ``match_first_index`` that :cpp:class:`Indices` already holds, and a WWMI fixer replaces it
     the same way -- reverse-then-forward through :cpp:func:`ModMappedAssets::replace`
     @endrst
     */
    class IndexCounts: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit IndexCounts(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
