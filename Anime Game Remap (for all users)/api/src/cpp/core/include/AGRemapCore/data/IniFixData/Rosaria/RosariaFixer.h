#ifndef AGRemapCore_RosariaFixer_H
#define AGRemapCore_RosariaFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Rosaria's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`RosariaParser`
     @endrst
     */
    class RosariaFixer {
        public:

            RosariaFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era Rosaria ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::rosaria6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
