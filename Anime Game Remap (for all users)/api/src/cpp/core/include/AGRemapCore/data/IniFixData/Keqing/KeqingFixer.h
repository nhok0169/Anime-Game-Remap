#ifndef AGRemapCore_KeqingFixer_H
#define AGRemapCore_KeqingFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Keqing's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`KeqingParser`, and read it next to
     :cpp:class:`KeqingOpulentFixer` --- the two are exact mirrors, and the pair is the worked
     example of a **merge** and the **split** that undoes it. Keqing draws a ``dress``
     KeqingOpulent has no geometry for, so remapping onto the skin collapses two objects into one
     and remapping back expands one into two
     @endrst
     */
    class KeqingFixer {
        public:

            KeqingFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era Keqing ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::keqing6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
