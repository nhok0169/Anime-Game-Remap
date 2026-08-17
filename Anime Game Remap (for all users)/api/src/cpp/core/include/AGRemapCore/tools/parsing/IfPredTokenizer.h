#ifndef AGRemapCore_IfPredTokenizer_H
#define AGRemapCore_IfPredTokenizer_H

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
