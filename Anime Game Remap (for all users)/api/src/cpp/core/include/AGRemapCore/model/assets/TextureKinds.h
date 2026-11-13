#ifndef AGRemapCore_TextureKinds_H
#define AGRemapCore_TextureKinds_H
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
     What each of a character's texture roles IS, per game version -- a :cpp:class:`ModMappedAssets` pre-populated with this project's real data
     (:cpp:func:`Data::getTextureKindDataRows`) :raw-html:`<br />` :raw-html:`<br />`

     Four index columns -- ``version`` (at position 0), ``name``, ``component`` and ``type`` -- as
     :cpp:class:`Indices` has, so :cpp:class:`ModMappedAssets`'s query methods take three non-version values
     positionally, in that order. A lookup at a version answers with the newest row at or below it
     :raw-html:`<br />` :raw-html:`<br />`

     The ``component`` is the draw slot the role is bound on (``component3``), or ``""`` for a role no one slot owns;
     the ``type`` is the role (``upperDiffuse``); the value is its KIND (``Diffuse``), or ``""`` for a role with no kind
     (a game texture such as ``ramp``). Two roles of one kind stand in for each other where a fix binds by kind --
     see :cpp:func:`ModType::getTextureKind` and :cpp:member:`WWMIFixerConfig::roleKinds` :raw-html:`<br />` :raw-html:`<br />`

     Kinds are one vocabulary across characters: ``Diffuse`` on one character is the same thing as on another
     @endrst
     */
    class TextureKinds: public ModMappedAssets<std::string, std::string> {
        public:
            /**
             * @brief Constructs a new, fully-populated lookup table
             *
             * @param map The `adjacency list`_ mapping the values to fix **from** to the values to fix **to**
             */
            explicit TextureKinds(std::unordered_map<std::string, std::vector<std::string>> map = {});
    };
}

#endif
