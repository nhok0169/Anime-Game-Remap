#ifndef AGRemapCore_YaoyaoParser_H
#define AGRemapCore_YaoyaoParser_H

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
     Yaoyao's own ``.ini`` parsers :raw-html:`<br />` :raw-html:`<br />`

     The standard GIMI character shape, drawing ``head`` / ``body`` -- both on the plain texture layout. Pair
     this with :cpp:class:`YaoyaoFixer`
     @endrst
     */
    class YaoyaoParser {
        public:

            YaoyaoParser() = delete;

            /**
             * @brief The parser for a 4.0-era Yaoyao ``.ini`` file -- what :cpp:func:`IniParseBuilderFuncs::yaoyao4_0` returns
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
