#ifndef AGRemapCore_CitlaliWhisperofStarsFixer_H
#define AGRemapCore_CitlaliWhisperofStarsFixer_H

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
     CitlaliWhisperofStars' own fixers :raw-html:`<br />` :raw-html:`<br />`

     The THIRD remap of a skin of SEVERAL components onto a target of ONE, and the inverse of
     :cpp:class:`CitlaliFixer`'s direction. See :cpp:func:`makeGIMIMergeFixer`. Pair this with
     :cpp:class:`CitlaliWhisperofStarsParser` :raw-html:`<br />` :raw-html:`<br />`

     Two things here that YelanTranquil's and BennettAdventure's directions do not need, both
     default-off options of the template rather than anything of Citlali's own:
     ``targetLayout`` (her shader reads three registers under ``ORFix``, where theirs read two
     under ``NNFix``) and ``texRegsByName`` (the skin's mods are written in the GAME's register
     order, and ``ORFix`` reads a different one)
     @endrst
     */
    class CitlaliWhisperofStarsFixer {
        public:

            CitlaliWhisperofStarsFixer() = delete;

            /**
             * @brief The fix onto Citlali -- what :cpp:func:`IniFixBuilderFuncs::citlaliWhisperofStarsToCitlali6_7` returns
             */
            static IniFixBuilder::Factory toCitlali6_7();
    };
}

#endif
