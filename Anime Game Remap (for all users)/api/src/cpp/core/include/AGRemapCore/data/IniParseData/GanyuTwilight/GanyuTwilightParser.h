#ifndef AGRemapCore_GanyuTwilightParser_H
#define AGRemapCore_GanyuTwilightParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     GanyuTwilight's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`GanyuTwilightFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     GanyuTwilight and Ganyu are a base/skin pair and remap onto each other
     @endrst
     */
    class GanyuTwilightParser {
        public:

            GanyuTwilightParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.4-era GanyuTwilight ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::ganyuTwilight4_4` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_4();

            /**
             * @brief
             @rst
             The parser for a 5.7-era GanyuTwilight ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::ganyuTwilight5_7` returns :raw-html:`<br />`
             :raw-html:`<br />`

             **This is the one a 6.1 fix resolves to**, the lookup taking the newest row at or below
             the version asked for
             @endrst
             */
            static IniParseBuilder::Factory v5_7();

    };
}

#endif
