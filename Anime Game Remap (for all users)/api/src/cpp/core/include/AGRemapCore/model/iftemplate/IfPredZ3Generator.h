#ifndef AGRemapCore_IfPredZ3Generator_H
#define AGRemapCore_IfPredZ3Generator_H

#include <string>

#include "AGRemapCore/tools/parsing/ParseTree.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The `Z3`_ predicate generator used for conditional predicates within a .ini file :raw-html:`<br />`
     :raw-html:`<br />`

     Walks the parse tree produced by :cpp:class:`IfPredParser` (the same tree shape the
     pure-Python ``IfPredLogicGenerator`` walks to build a `sympy`_ logic query) and builds the
     equivalent boolean-sorted `Z3`_ expression instead

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
    class IfPredZ3Generator {
        public:

            /**
             * @brief Generates a `Z3`_ predicate from the parse tree
             *
             * @param parseTree The tree to parse
             *
             * @param ctx The `Z3`_ context the generated predicate will belong to
             *
             * @param simplify Whether to simplify the resultant predicate :raw-html:`<br />`
             *      :raw-html:`<br />`
             *
             *      **Default**: ``true``
             *
             * @return The generated `Z3`_ predicate
             */
            static Z3Predicate generate(const ParseTree<std::string>& parseTree, Z3Context& ctx, bool simplify = true);
    };
}

#endif
