#ifndef AGRemapCore_Z3Predicate_H
#define AGRemapCore_Z3Predicate_H

#include <memory>
#include <string>

#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     An opaque wrapper around a boolean-sorted `Z3`_ expression (``z3::expr``) :raw-html:`<br />`
     :raw-html:`<br />`

     Produced by :cpp:func:`IfPredZ3Generator::generate`. Like :cpp:class:`Z3Context`, this class
     never exposes any `Z3`_ type in its own header -- the real ``z3::expr`` lives behind a pimpl
     (:cpp:class:`Impl`, defined only in this class's own ``.cpp``)
     @endrst
     */
    class Z3Predicate {
        public:

            ~Z3Predicate();

            Z3Predicate(const Z3Predicate&);
            Z3Predicate& operator=(const Z3Predicate&);

            Z3Predicate(Z3Predicate&&) noexcept;
            Z3Predicate& operator=(Z3Predicate&&) noexcept;

            /**
             * @brief The predicate rendered as a `Z3`_ SMT-LIB2 expression string (the same text
             *      ``z3::expr::to_string()`` would produce)
             *
             * @return The string form of the predicate
             */
            std::string toString() const;

            /**
             * @brief
             @rst
             Logical AND with another predicate :raw-html:`<br />` :raw-html:`<br />`

             Both predicates must belong to the same :cpp:class:`Z3Context` (see #belongsTo) --
             combining predicates from two different contexts is a `Z3`_-level precondition
             violation (only checked via ``assert`` in `Z3`_'s own C++ API, so it is **not**
             guaranteed to throw in a release build -- callers that cannot already guarantee a
             shared context must check #belongsTo/#sameContext first, or reparent one side via
             :cpp:func:`IfPredPart::reparent`)
             @endrst
             *
             * @param other The predicate to combine with
             *
             * @return The combined predicate, in this predicate's own context
             */
            Z3Predicate operator&(const Z3Predicate& other) const;

            /**
             * @brief
             @rst
             Logical OR with another predicate -- see #operator& for the same-context precondition
             @endrst
             *
             * @param other The predicate to combine with
             *
             * @return The combined predicate, in this predicate's own context
             */
            Z3Predicate operator|(const Z3Predicate& other) const;

            /**
             * @brief Logical negation of this predicate
             *
             * @return The negated predicate, in this predicate's own context
             */
            Z3Predicate operator!() const;

            /**
             * @brief
             @rst
             A simplified, logically-equivalent form of this predicate (`Z3`_'s own
             ``z3::expr::simplify()``) :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             *
             * @return The simplified predicate
             */
            Z3Predicate simplify() const;

            /**
             * @brief
             @rst
             Whether this predicate is satisfiable -- ie. whether some assignment of its free
             variables makes it evaluate to ``true``, checked via a real ``z3::solver`` :raw-html:`<br />`
             :raw-html:`<br />`
             @endrst
             *
             * @return Whether this predicate is satisfiable
             */
            bool isSatisfiable() const;

            /**
             * @brief
             @rst
             Whether 'other' belongs to the same :cpp:class:`Z3Context` as this predicate (a plain
             identity check on the underlying ``z3::context``, not a check of logical equivalence)
             @endrst
             *
             * @param other The predicate to compare against
             *
             * @return Whether both predicates share the same underlying `Z3`_ context
             */
            bool sameContext(const Z3Predicate& other) const;

            /**
             * @brief
             @rst
             Whether this predicate belongs to 'ctx' (a plain identity check on the underlying
             ``z3::context``)
             @endrst
             *
             * @param ctx The context to compare against
             *
             * @return Whether this predicate belongs to 'ctx'
             */
            bool belongsTo(const Z3Context& ctx) const;

            /**
             * @brief The literal ``true`` predicate, in the given `Z3`_ context
             *
             * @param ctx The context the returned predicate will belong to
             *
             * @return The literal ``true`` predicate
             */
            static Z3Predicate trueValue(Z3Context& ctx);

            /**
             * @brief The literal ``false`` predicate, in the given `Z3`_ context
             *
             * @param ctx The context the returned predicate will belong to
             *
             * @return The literal ``false`` predicate
             */
            static Z3Predicate falseValue(Z3Context& ctx);

            /**
             * @brief The real, hidden implementation for this class (holds the actual, boolean-sorted
             *      ``z3::expr``) -- only ever defined/accessed from within the small set of ``.cpp``
             *      files that are allowed to know about `Z3`_ directly (this class's own ``.cpp``,
             *      :cpp:class:`IfPredZ3Generator`'s/:cpp:class:`Z3IfPredGenerator`'s, and
             *      :cpp:class:`IfPredPart`'s)
             */
            class Impl;

        private:
            explicit Z3Predicate(std::unique_ptr<Impl> impl);

            std::unique_ptr<Impl> impl_;

            Impl& impl();
            const Impl& impl() const;

            friend class IfPredZ3Generator;
            friend class Z3IfPredGenerator;

            // IfPredPart needs to build a literal 'true' Z3Predicate directly for
            // IfPredPartType::Else (no parse tree to run through IfPredZ3Generator for that
            // trivial case).
            friend class IfPredPart;

            // Test/debug-only construction hook, defined in the private (never-installed)
            // Z3Internal.h -- lets a core/tests/ verification harness hand-build a Z3Predicate
            // directly from an arbitrary already-boolean expr (wrapped in an Impl, so this
            // declaration itself still never needs to name any real Z3 type), without opening up
            // the constructor to any real (non-test) caller. See Z3Internal.h's own comment.
            friend Z3Predicate makeZ3PredicateForTesting(std::unique_ptr<Impl> impl);

            // Test/debug-only access hook -- lets a core/tests/ verification harness reach the
            // real z3::expr (eg. to build a z3::solver directly for a semantic-equivalence
            // check), without opening up #impl to any real (non-test) caller.
            friend const Impl& getZ3PredicateImplForTesting(const Z3Predicate& predicate);
    };
}

#endif
