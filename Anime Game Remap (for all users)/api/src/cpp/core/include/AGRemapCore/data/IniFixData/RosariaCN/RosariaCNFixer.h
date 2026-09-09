#ifndef AGRemapCore_RosariaCNFixer_H
#define AGRemapCore_RosariaCNFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     RosariaCN's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`RosariaCNParser`
     @endrst
     */
    class RosariaCNFixer {
        public:

            RosariaCNFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era RosariaCN ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::rosariaCN6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
