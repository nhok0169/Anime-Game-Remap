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

#ifndef AGRemapCore_NeuvilletteMelusentParser_H
#define AGRemapCore_NeuvilletteMelusentParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     NeuvilletteMelusent's own ``.ini`` parsers :raw-html:`<br />` :raw-html:`<br />`

     A skin of FOUR components -- an UNNAMED main mesh (component ``""``: Head, Body, Dress), a ``Coat``, a
     ``Bang`` and an ``Eye`` -- see :cpp:func:`makeGIMIComponentParser`. Pair this with
     :cpp:class:`NeuvilletteMelusentFixer`
     @endrst
     */
    class NeuvilletteMelusentParser {
        public:

            NeuvilletteMelusentParser() = delete;

            /**
             * @brief The parser for a 6.3-era NeuvilletteMelusent ``.ini`` file -- what :cpp:func:`IniParseBuilderFuncs::neuvilletteMelusent6_3` returns
             */
            static IniParseBuilder::Factory v6_3();
    };
}

#endif
