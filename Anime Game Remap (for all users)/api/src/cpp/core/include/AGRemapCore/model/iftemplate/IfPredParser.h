#ifndef AGRemapCore_IfPredParser_H
#define AGRemapCore_IfPredParser_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

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
