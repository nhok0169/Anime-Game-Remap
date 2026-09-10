#ifndef AGRemapCore_JeanSeaFixer_H
#define AGRemapCore_JeanSeaFixer_H

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
     JeanSea's own ``.ini`` fixers -- the **merge** side of the Jean family, and the only shape here
     that writes more than one ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     JeanSea draws a ``dress`` (her cape) that neither Jean nor JeanCN has any geometry for, so
     remapping *off* her is the reverse of :cpp:class:`JeanFixer`'s split: her ``body`` **and** her
     ``dress`` both have to become the target's single ``body``

     .. code-block::

        JeanSea          Jean
        head    ----->   head
        body    --+-->   body
        dress   --+

     Two sections cannot both be named ``body`` in one file -- a GIMI-like importer warns about two
     sections on one hash and shows only one of them. The way out is the importer's own
     overlapping-mod behaviour: the fix emits a **second ``.ini`` file** holding the other
     half, and the game draws both. :cpp:class:`GraphGroupRemap` produces that second
     :cpp:class:`IniGraphGroup` on its own, simply because the two targets collide
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        **Both files carry the whole non-drawn set** -- blend, position, texcoord, ib,
        VertexLimitRaise and face -- not just the merged object. A second file naming a
        ``Blend.buf`` that is not in it would be broken; the old script's own
        ``JeanSeaRemapFix1.ini`` shows the full set

     Like Jean, JeanSea is **two fixers, one per target**, rather than a ``MultiModFixer`` -- see
     :cpp:class:`JeanFixer`. Pair these with :cpp:class:`JeanSeaParser`
     @endrst
     */
    class JeanSeaFixer {
        public:

            JeanSeaFixer() = delete;

            /**
             * @brief
             @rst
             The 6.1 fix remapping JeanSea onto **Jean** -- what
             :cpp:func:`IniFixBuilderFuncs::jeanSea6_1ToJean` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1ToJean();

            /**
             * @brief
             @rst
             The 6.1 fix remapping JeanSea onto **JeanCN** -- what
             :cpp:func:`IniFixBuilderFuncs::jeanSea6_1ToJeanCN` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1ToJeanCN();
    };
}

#endif
