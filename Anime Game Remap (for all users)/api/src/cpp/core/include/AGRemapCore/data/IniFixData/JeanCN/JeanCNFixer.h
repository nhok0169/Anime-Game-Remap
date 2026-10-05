#ifndef AGRemapCore_JeanCNFixer_H
#define AGRemapCore_JeanCNFixer_H

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
     JeanCN's own ``.ini`` fixers -- **two per game version**, one per character she remaps onto
     :raw-html:`<br />` :raw-html:`<br />`

     JeanCN has the same two-fixer shape Jean does, and for the same reason whose fix is not a single fixer, and the reason is that her
     two targets are genuinely different shapes:

     ================  ==============================================================================
     Target            What the fix has to do
     ================  ==============================================================================
     ``Jean``          the ordinary skin remap -- a different model, so hashes and indices are
                       replaced and the originals left alone. The same shape as MonaCN -> Mona
     ``JeanSea``       the same, **plus a split**: JeanSea draws a ``dress`` (her cape) that JeanCN
                       simply does not have, so JeanCN's ``body`` graph is emitted twice, once as
                       JeanSea's ``body`` and once as her ``dress``
     ================  ==============================================================================

     .. note::
        **The pure-Python original expressed this as one ``MultiModFixer`` holding a
        ``{target -> fixer}`` map.** That indirection is unnecessary here:
        :cpp:class:`IniFixBuilderData`'s table is keyed by ``(from mod, to mod)`` already, so the
        two fixers are simply two rows. :cpp:class:`MultiModFixer` still exists for fixes that
        genuinely need one fixer to run over another's output

     Pair these with :cpp:class:`JeanCNParser`, which is shared between them
     @endrst
     */
    class JeanCNFixer {
        public:

            JeanCNFixer() = delete;

            /**
             * @brief
             @rst
             The 6.1 fix remapping JeanCN onto **Jean** -- what
             :cpp:func:`IniFixBuilderFuncs::jeanCN6_1ToJean` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1ToJean();

            /**
             * @brief
             @rst
             The 6.1 fix remapping JeanCN onto **JeanSea** -- what
             :cpp:func:`IniFixBuilderFuncs::jeanCN6_1ToJeanSea` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1ToJeanSea();
    };
}

#endif
