#ifndef AGRemapCore_ShenheParser_H
#define AGRemapCore_ShenheParser_H

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Shenhe's own ``.ini`` parsers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`ShenheFixer`; the two agree on the mod object names by hand, and
     neither half makes sense alone :raw-html:`<br />` :raw-html:`<br />`

     Shenhe and ShenheFrostFlower are a base/skin pair and remap onto each other. The
     skin draws a fourth object (``extra``) that Shenhe does not have
     @endrst
     */
    class ShenheParser {
        public:

            ShenheParser() = delete;

            /**
             * @brief
             @rst
             The parser for a 4.0-era Shenhe ``.ini`` file -- what
             :cpp:func:`IniParseBuilderFuncs::shenhe4_0` returns, and documented there
             @endrst
             */
            static IniParseBuilder::Factory v4_0();
    };
}

#endif
