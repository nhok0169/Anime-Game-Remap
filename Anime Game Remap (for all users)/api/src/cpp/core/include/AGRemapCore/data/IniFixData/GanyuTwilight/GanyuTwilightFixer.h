#ifndef AGRemapCore_GanyuTwilightFixer_H
#define AGRemapCore_GanyuTwilightFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     GanyuTwilight's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`GanyuTwilightParser`
     @endrst
     */
    class GanyuTwilightFixer {
        public:

            GanyuTwilightFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era GanyuTwilight ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::ganyuTwilight6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
