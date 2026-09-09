#ifndef AGRemapCore_GanyuParser_H
#define AGRemapCore_GanyuParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Ganyu's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`GanyuFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     Ganyu and GanyuTwilight are a base/skin pair and remap onto each other
     @endrst
     */
    class GanyuParser {
        public:

            GanyuParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Ganyu ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::ganyu4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();

    };
}

#endif
