#ifndef AGRemapCore_IfPredParser_H
#define AGRemapCore_IfPredParser_H

#include <string>

#include "AGRemapCore/tools/parsing/BaseSLR1Parser.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseSLR1Parser`

     The context-free parser used for conditional predicates within a .ini file

     eg.

     .. code-block:: ini
         :linenos:
         :emphasize-lines: 1,3

         if pred1
             ...
         else if pred2
             ...
         endif
     @endrst
     */
    class IfPredParser: public BaseSLR1Parser<std::string> {
        public:

            /**
             * @brief Constructs a new if-predicate parser
             *
             * @param startToken The name of the starting token for an input string
             * @param endToken The name of the ending token for an input string
             * @param nullToken The name for the empty token
             * @param setup Whether to initialize all the setup for the parser automatically by calling #setup
             */
            explicit IfPredParser(std::string startToken = "STARTTOKEN", std::string endToken = "ENDTOKEN", std::string nullToken = "EPSILON",
                                   bool setup = true);
    };
}

#endif
