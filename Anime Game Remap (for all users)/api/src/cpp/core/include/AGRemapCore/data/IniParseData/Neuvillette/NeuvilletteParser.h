#ifndef AGRemapCore_NeuvilletteParser_H
#define AGRemapCore_NeuvilletteParser_H

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
     Neuvillette's own ``.ini`` parsers :raw-html:`<br />` :raw-html:`<br />`

     The standard GIMI character shape, drawing ``head`` / ``body`` / ``dress`` -- his head and dress on
     the plain texture layout, his body on the normal-map one, and his mods not agreeing with either. Pair
     this with :cpp:class:`NeuvilletteFixer`
     @endrst
     */
    class NeuvilletteParser {
        public:

            NeuvilletteParser() = delete;

            /**
             * @brief The parser for a 4.0-era Neuvillette ``.ini`` file -- what :cpp:func:`IniParseBuilderFuncs::neuvillette4_0` returns
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
