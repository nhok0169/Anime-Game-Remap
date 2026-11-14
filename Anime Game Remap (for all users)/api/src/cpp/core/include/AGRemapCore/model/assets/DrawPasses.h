#ifndef AGRemapCore_DrawPasses_H
#define AGRemapCore_DrawPasses_H
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
     The draw passes of a WWMI character's draw slots, per game version -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getDrawPassDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     Four index columns -- ``version`` (at position 0), ``name``, ``component`` and ``type`` -- as
     :cpp:class:`Indices` has, so :cpp:class:`ModMappedAssets`'s query methods take three non-version values
     positionally, in that order. A lookup at a version answers with the newest row at or below it
     :raw-html:`<br />` :raw-html:`<br />`

     The slot is the ``component`` (``component0``, ``component1``, ...) and the ``type`` names a pass and
     what is read of it: ``<pass>`` -- the vertex shaders the pass draws with, comma-separated; ``<pass>.ps`` -- its
     pixel shader; ``<pass>.tags`` -- the ``filter_index`` a fix tags each of those vertex shaders with, in the same
     order. The passes are ``main`` (``main2``, ... for a slot drawn more than once) and ``other`` (an outline, a
     second pass) :raw-html:`<br />` :raw-html:`<br />`

     A fix gates what it binds on its target slot's passes. A game update that changes a pass's shader is a new
     row at the new version; a fix asked for an older version still reads that version's rows
     @endrst
     */
    class DrawPasses: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit DrawPasses(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
