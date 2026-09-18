#ifndef AGRemapCore_CherryHuTaoFixer_H
#define AGRemapCore_CherryHuTaoFixer_H

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
     CherryHuTao's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`CherryHuTaoParser`, and read it next to :cpp:class:`HuTaoFixer`
     :raw-html:`<br />` :raw-html:`<br />`

     The most involved fix here: a MERGE of four objects onto two, three texture edits,
     reflection sections to strip, and a register shift that belongs to only half the
     copies -- see :cpp:member:`GIMICharFixerConfig::srcObjRegRemaps`
     @endrst
     */
    class CherryHuTaoFixer {
        public:

            CherryHuTaoFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era CherryHuTao ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::cherryHuTao6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
