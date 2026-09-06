#ifndef AGRemapCore_AmberParser_H
#define AGRemapCore_AmberParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Amber's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Split out of :cpp:class:`IniParseBuilderData`'s own translation unit for the reason
     :cpp:class:`RaidenParser` is -- a real generator is an order of magnitude larger than the stub
     it replaces. :cpp:class:`IniParseBuilderFuncs` still declares the entry point; only the
     *definition* lives here
     @endrst
     */
    class AmberParser {
        public:

            AmberParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Amber ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::amber4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
