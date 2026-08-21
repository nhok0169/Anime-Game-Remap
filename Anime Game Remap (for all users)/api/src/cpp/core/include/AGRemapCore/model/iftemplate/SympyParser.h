#ifndef AGRemapCore_SympyParser_H
#define AGRemapCore_SympyParser_H

#include <string>

#include "AGRemapCore/tools/parsing/BaseSLR1Parser.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseSLR1Parser`

     The context-free parser used for a subset of the string representation of a `sympy logic
     query`_

     eg.

     .. code-block:: ini
         :linenos:

         ~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))
     @endrst
     */
    class SympyParser: public BaseSLR1Parser<std::string> {
        public:

            /**
             * @brief Constructs a new sympy query parser
             *
             * @param startToken The name of the starting token for an input string
             * @param endToken The name of the ending token for an input string
             * @param nullToken The name for the empty token
             * @param setup Whether to initialize all the setup for the parser automatically by calling #setup
             */
            explicit SympyParser(std::string startToken = "STARTTOKEN", std::string endToken = "ENDTOKEN", std::string nullToken = "EPSILON",
                                  bool setup = true);
    };
}

#endif
