#ifndef AGRemapCore_JeanSeaParser_H
#define AGRemapCore_JeanSeaParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     JeanSea's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`JeanSeaFixer`. JeanSea is the MERGE side of the Jean family
     rather than one -- she remaps onto JeanCN *and* onto JeanSea, and those are different shapes --
     but the parser is shared between them, because what a Jean ``.ini`` file *contains* does not
     depend on what it is being remapped to
     @endrst
     */
    class JeanSeaParser {
        public:

            JeanSeaParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Jean ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::jeanSea4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();

    };
}

#endif
