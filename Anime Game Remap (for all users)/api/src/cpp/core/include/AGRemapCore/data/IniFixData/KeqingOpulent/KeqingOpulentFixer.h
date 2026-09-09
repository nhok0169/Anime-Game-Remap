#ifndef AGRemapCore_KeqingOpulentFixer_H
#define AGRemapCore_KeqingOpulentFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     KeqingOpulent's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`KeqingOpulentParser`, and read it next to :cpp:class:`KeqingFixer`
     --- the two are exact mirrors, this one being the **split** that undoes the other's merge
     @endrst
     */
    class KeqingOpulentFixer {
        public:

            KeqingOpulentFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era KeqingOpulent ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::keqingOpulent6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
