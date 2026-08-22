#ifndef AGRemapCore_Z3IfPredGenerator_H
#define AGRemapCore_Z3IfPredGenerator_H

#include <string>

#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The conditional predicate generator for .ini files, using a `Z3`_ predicate :raw-html:`<br />`
     :raw-html:`<br />`

     The inverse of :cpp:class:`IfPredZ3Generator` -- rather than a pure-Python ``sympy``-flavored
     `sympy logic query`_ generator (see ``SympyIfPredGenerator.py``'s own docstring for that
     eg.), this walks an arbitrary `Z3`_ expression directly (there is no `CFG`_-shaped parse tree
     on this side, since a ``z3::expr`` is a plain expression DAG with no grammar behind it) and
     renders the equivalent .ini predicate text

     eg. (produced from ``And(Eq($x$, $y$), Not($z$))``-shaped predicate)

     .. code-block:: ini
         :linenos:

         $x == $y && !$z
     @endrst
     */
    class Z3IfPredGenerator {
        public:

            /**
             * @brief Generates the .ini predicate text equivalent to a `Z3`_ predicate
             *
             * @param predicate The predicate to render
             *
             * @throws std::invalid_argument If 'predicate' contains a construct with no .ini
             *      predicate equivalent (eg. a quantifier, a non-Real/Bool sort, or a general
             *      ``ite`` that isn't the ``ite(boolExpr, 1, 0)`` shape :cpp:func:`IfPredZ3Generator`
             *      itself produces when coercing a boolean operand to an arithmetic one)
             *
             * @return The generated .ini predicate text
             */
            static std::string generate(const Z3Predicate& predicate);
    };
}

#endif
