#ifndef AGRemapCore_SympyTokenizer_H
#define AGRemapCore_SympyTokenizer_H

#include "AGRemapCore/tools/parsing/FilteredTokenizer.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`FilteredTokenizer`

     The tokenizer used for a subset of the string representation of a `sympy logic query`_

     eg. ``~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))``
     @endrst
     */
    class SympyTokenizer : public FilteredTokenizer {
        public:

            /**
             * @brief Constructs a new tokenizer
             *
             * @param setup Whether to initialize all the setup for the tokenizer automatically by calling :cpp:func:`setup`
             */
            explicit SympyTokenizer(bool setup = true);

        protected:
            void addStates() override;
            void addTransitions() override;
    };
}

#endif
