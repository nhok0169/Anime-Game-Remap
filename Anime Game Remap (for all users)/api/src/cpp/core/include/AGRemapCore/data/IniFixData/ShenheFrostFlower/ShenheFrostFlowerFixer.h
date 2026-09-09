#ifndef AGRemapCore_ShenheFrostFlowerFixer_H
#define AGRemapCore_ShenheFrostFlowerFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     ShenheFrostFlower's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`ShenheFrostFlowerParser`, and read it next to
     :cpp:class:`ShenheFixer`. This is the **widest merge** in the repo: three of the skin's
     objects land on Shenhe's one ``body``, so the fix writes THREE ``.ini`` files
     @endrst
     */
    class ShenheFrostFlowerFixer {
        public:

            ShenheFrostFlowerFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era ShenheFrostFlower ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::shenheFrostFlower6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
