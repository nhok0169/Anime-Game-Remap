#ifndef AGRemapCore_AmberFixer_H
#define AGRemapCore_AmberFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Amber's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`AmberParser`
     @endrst
     */
    class AmberFixer {
        public:

            AmberFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era Amber ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::amber6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
