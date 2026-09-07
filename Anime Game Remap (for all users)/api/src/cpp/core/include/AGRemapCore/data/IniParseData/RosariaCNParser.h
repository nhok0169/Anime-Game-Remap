#ifndef AGRemapCore_RosariaCNParser_H
#define AGRemapCore_RosariaCNParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     RosariaCN's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`RosariaCNFixer`: the fixer's mod objects are the ones this parser
     classifies, and neither half makes sense alone
     @endrst
     */
    class RosariaCNParser {
        public:

            RosariaCNParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era RosariaCN ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::rosariaCN4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
