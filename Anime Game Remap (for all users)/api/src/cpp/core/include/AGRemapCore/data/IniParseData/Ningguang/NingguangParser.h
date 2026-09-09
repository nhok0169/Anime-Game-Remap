#ifndef AGRemapCore_NingguangParser_H
#define AGRemapCore_NingguangParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Ningguang's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`NingguangFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     Ningguang and NingguangOrchid are a base/skin pair and remap onto each other
     @endrst
     */
    class NingguangParser {
        public:

            NingguangParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Ningguang ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::ningguang4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();

    };
}

#endif
