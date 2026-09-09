#ifndef AGRemapCore_MonaFixer_H
#define AGRemapCore_MonaFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Mona's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`MonaParser`
     @endrst
     */
    class MonaFixer {
        public:

            MonaFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era Mona ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::mona6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
