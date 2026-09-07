#ifndef AGRemapCore_AmberCNFixer_H
#define AGRemapCore_AmberCNFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     AmberCN's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`AmberCNParser`
     @endrst
     */
    class AmberCNFixer {
        public:

            AmberCNFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era AmberCN ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::amberCN6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
