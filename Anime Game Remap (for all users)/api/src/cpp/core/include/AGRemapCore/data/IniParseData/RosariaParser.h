#ifndef AGRemapCore_RosariaParser_H
#define AGRemapCore_RosariaParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Rosaria's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`RosariaFixer`: the fixer's mod objects are the ones this parser
     classifies, and neither half makes sense alone
     @endrst
     */
    class RosariaParser {
        public:

            RosariaParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Rosaria ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::rosaria4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
