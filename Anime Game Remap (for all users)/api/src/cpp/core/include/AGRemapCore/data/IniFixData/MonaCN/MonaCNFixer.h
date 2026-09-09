#ifndef AGRemapCore_MonaCNFixer_H
#define AGRemapCore_MonaCNFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     MonaCN's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`MonaCNParser`
     @endrst
     */
    class MonaCNFixer {
        public:

            MonaCNFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era MonaCN ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::monaCN6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
