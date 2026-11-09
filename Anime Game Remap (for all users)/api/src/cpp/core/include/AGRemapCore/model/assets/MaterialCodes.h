#ifndef AGRemapCore_MaterialCodes_H
#define AGRemapCore_MaterialCodes_H
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
     What the codes of a character's single-channel code maps mean, per game version -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getMaterialCodeDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     Four index columns -- ``version`` (at position 0), ``name``, ``component`` and ``type`` -- as
     :cpp:class:`Indices` has, so :cpp:class:`ModMappedAssets`'s query methods take three non-version values
     positionally, in that order. A lookup at a version answers with the newest row at or below it
     :raw-html:`<br />` :raw-html:`<br />`

     The ``component`` is the draw slot (``component6``) and the ``type`` names a code map role and what of it:
     ``<role>.<field>`` -- the bits a field of the code occupies (``propsDetail.flags.film``: ``0x10``;
     ``propsDetail.material``: ``0x0f``); ``<role>.<field>.<name>`` -- one code of a field (``pouchDetail.material.plain``:
     ``15``). Values are numbers, decimal or ``0x`` hex -- see :cpp:func:`ModType::getMaterialCode` :raw-html:`<br />` :raw-html:`<br />`

     A game update that renumbers a character's materials is new rows at the new version, not an edit to a filter
     @endrst
     */
    class MaterialCodes: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit MaterialCodes(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
