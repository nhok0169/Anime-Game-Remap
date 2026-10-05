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

#ifndef AGRemapCore_YaoyaoBambooParser_H
#define AGRemapCore_YaoyaoBambooParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     YaoyaoBamboo's own ``.ini`` parsers :raw-html:`<br />` :raw-html:`<br />`

     A skin of THREE components -- an UNNAMED main mesh (component ``""``: Head, Body), a ``Bang`` and an
     ``Eye`` -- see :cpp:func:`makeGIMIComponentParser`. Pair this with :cpp:class:`YaoyaoBambooFixer`
     @endrst
     */
    class YaoyaoBambooParser {
        public:

            YaoyaoBambooParser() = delete;

            /**
             * @brief The parser for a 6.3-era YaoyaoBamboo ``.ini`` file -- what :cpp:func:`IniParseBuilderFuncs::yaoyaoBamboo6_3` returns
             */
            static IniParseBuilder::Factory v6_3();
    };
}

#endif
