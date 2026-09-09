#ifndef AGRemapCore_NingguangFixer_H
#define AGRemapCore_NingguangFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Ningguang's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`NingguangParser`
     @endrst
     */
    class NingguangFixer {
        public:

            NingguangFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era Ningguang ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::ningguang6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
