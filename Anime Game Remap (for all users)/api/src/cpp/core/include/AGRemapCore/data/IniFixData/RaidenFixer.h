#ifndef AGRemapCore_RaidenFixer_H
#define AGRemapCore_RaidenFixer_H

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Raiden's own ``.ini`` fixers, one per game version she needed a new one at
     :raw-html:`<br />` :raw-html:`<br />`

     Split out of :cpp:class:`IniFixBuilderData`'s own translation unit for the same reason
     :cpp:class:`RaidenParser` is -- a real generator is a fixer subclass plus every edit it owns,
     where the stub it replaces is one line. :cpp:class:`IniFixBuilderFuncs` still declares the
     entry point; only the *definition* lives here :raw-html:`<br />` :raw-html:`<br />`

     Pair this with :cpp:class:`RaidenParser`: the fixer's mod objects are the ones that parser
     classifies, and neither half makes sense alone
     @endrst
     */
    class RaidenFixer {
        public:

            RaidenFixer() = delete;

            /**
             * @brief
             @rst
             The fixer for a 6.1-era Raiden ``.ini`` file -- what
             :cpp:func:`IniFixBuilderFuncs::raiden6_1` returns, and documented there
             @endrst
             */
            static IniFixBuilder::Factory v6_1();
    };
}

#endif
