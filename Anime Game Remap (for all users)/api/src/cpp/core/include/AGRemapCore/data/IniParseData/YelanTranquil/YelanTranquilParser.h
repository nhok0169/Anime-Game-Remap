#ifndef AGRemapCore_YelanTranquilParser_H
#define AGRemapCore_YelanTranquilParser_H

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
     YelanTranquil's own ``.ini`` parsers :raw-html:`<br />` :raw-html:`<br />`

     The FIRST parser for a skin of several components -- a ``Body`` drawing three slots, a ``Bang``
     and an ``Eye``, each with its own buffers and its own hashes. See
     :cpp:func:`makeGIMIComponentParser`. Pair this with :cpp:class:`YelanTranquilFixer`
     @endrst
     */
    class YelanTranquilParser {
        public:

            YelanTranquilParser() = delete;

            /**
             * @brief The parser for a 5.7-era YelanTranquil ``.ini`` file -- what :cpp:func:`IniParseBuilderFuncs::yelanTranquil5_7` returns
             */
            static IniParseBuilder::Factory v5_7();
    };
}

#endif
