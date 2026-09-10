#ifndef AGRemapCore_IfPredTokenizer_H
#define AGRemapCore_IfPredTokenizer_H

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

#include "AGRemapCore/tools/parsing/FilteredTokenizer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`FilteredTokenizer`

     The tokenizer used for conditional predicates within a .ini file

     eg. ``if pred1 ... else if pred2 ... endif``
     @endrst
     */
    class IfPredTokenizer : public FilteredTokenizer {
        public:

            /**
             * @brief Constructs a new tokenizer
             *
             * @param setup Whether to initialize all the setup for the tokenizer automatically by calling :cpp:func:`setup`
             */
            explicit IfPredTokenizer(bool setup = true);

        protected:
            void addStates() override;
            void addTransitions() override;
    };
}

#endif
