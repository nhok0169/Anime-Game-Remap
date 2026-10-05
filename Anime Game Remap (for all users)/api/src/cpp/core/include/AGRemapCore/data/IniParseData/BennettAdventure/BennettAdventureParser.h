#ifndef AGRemapCore_BennettAdventureParser_H
#define AGRemapCore_BennettAdventureParser_H

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

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     BennettAdventure's own ``.ini`` parsers -- the second skin of SEVERAL COMPONENTS
     :raw-html:`<br />` :raw-html:`<br />`

     A ``Body`` (draw slots ``A`` and ``B``), a ``Bang`` and an ``Eye``, each with its own buffers
     and its own vertex-group index space -- the YelanTranquil shape. See
     :cpp:func:`makeGIMIComponentParser`. Pair this with :cpp:class:`BennettAdventureFixer`
     @endrst
     */
    class BennettAdventureParser {
        public:

            BennettAdventureParser() = delete;

            /**
             * @brief The parser for a 5.7-era BennettAdventure ``.ini`` file -- what :cpp:func:`IniParseBuilderFuncs::bennettAdventure5_7` returns
             */
            static IniParseBuilder::Factory v5_7();
    };
}

#endif
