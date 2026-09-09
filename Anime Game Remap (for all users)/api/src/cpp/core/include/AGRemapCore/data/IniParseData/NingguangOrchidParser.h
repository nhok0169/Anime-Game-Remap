#ifndef AGRemapCore_NingguangOrchidParser_H
#define AGRemapCore_NingguangOrchidParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     NingguangOrchid's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`NingguangOrchidFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     NingguangOrchid and Ningguang are a base/skin pair and remap onto each other
     @endrst
     */
    class NingguangOrchidParser {
        public:

            NingguangOrchidParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era NingguangOrchid ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::ningguangOrchid4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();

    };
}

#endif
