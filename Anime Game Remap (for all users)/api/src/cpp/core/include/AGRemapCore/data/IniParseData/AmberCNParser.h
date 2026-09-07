#ifndef AGRemapCore_AmberCNParser_H
#define AGRemapCore_AmberCNParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     AmberCN's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`AmberCNFixer`: the fixer's mod objects are the ones this parser
     classifies, and neither half makes sense alone
     @endrst
     */
    class AmberCNParser {
        public:

            AmberCNParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era AmberCN ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::amberCN4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
