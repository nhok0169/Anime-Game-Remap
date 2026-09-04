#ifndef AGRemapCore_Z3Context_H
#define AGRemapCore_Z3Context_H

#include <memory>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     An opaque wrapper around a `Z3`_ context (``z3::context``) :raw-html:`<br />` :raw-html:`<br />`

     This class deliberately never exposes any `Z3`_ type in its own header (or anywhere else in
     :cpp:class:`AGRemapCore`'s public headers) -- the real ``z3::context`` lives behind a
     pimpl (:cpp:class:`Impl`, defined only in this class's own ``.cpp``), so that including this
     header (or any header that transitively includes it, eg. :cpp:class:`IfPredZ3Generator`'s)
     never requires ``z3++.h`` to be on the include path

     .. note::
        Every ``Z3Predicate`` built from a given ``Z3Context`` keeps that context's underlying
        ``z3::context`` alive via a shared reference, so a ``Z3Context`` going out of scope (or
        being garbage-collected in Python) while ``Z3Predicate``\\s built from it are still alive
        is safe, in any destruction order -- including predicates of several already-gone
        contexts being destroyed interleaved with each other. An earlier version of this comment
        described that interleaved case as a "known limitation of Z3's own C++ API"; it was in
        fact a member-declaration-order bug in ``Z3Predicate``'s pimpl (the keep-alive was
        destroyed *before* the ``z3::expr`` it protected, so the last predicate on a context
        ``dec_ref``'d into a freed context), fixed on 2026-09-03 and pinned by
        ``core/tests/Z3Predicate_MemberOrder_test.cpp``. Sharing one long-lived ``Z3Context`` per
        ``.ini`` file (``IniFile``'s own shape) is still the right design, since predicates from
        different contexts cannot be combined -- but it is no longer needed for memory safety.
     @endrst
     */
    class Z3Context {
        public:

            /**
             * @brief Constructs a new, empty Z3 context
             */
            Z3Context();

            ~Z3Context();

            Z3Context(const Z3Context&) = delete;
            Z3Context& operator=(const Z3Context&) = delete;

            Z3Context(Z3Context&&) noexcept;
            Z3Context& operator=(Z3Context&&) noexcept;

            /**
             * @brief The real, hidden implementation for this class (holds the actual
             *      ``z3::context``) -- only ever defined/accessed from within the small set of
             *      ``.cpp`` files that are allowed to know about `Z3`_ directly (this class's own
             *      ``.cpp``, :cpp:class:`IfPredZ3Generator`'s, and :cpp:class:`IfPredPart`'s)
             */
            class Impl;

        private:
            std::unique_ptr<Impl> impl_;

            Impl& impl();
            const Impl& impl() const;

            friend class IfPredZ3Generator;

            // IfPredPart needs the real z3::context directly to build a literal 'true'
            // Z3Predicate for IfPredPartType::Else (no parse tree to run through
            // IfPredZ3Generator for that trivial case).
            friend class IfPredPart;

            // Z3Predicate needs the real z3::context directly for its own trueValue()/falseValue()
            // static factories (building a literal Z3Predicate with no expression tree to walk),
            // and for belongsTo() (comparing this context's own underlying z3::context identity
            // against a Z3Predicate's -- see Z3Predicate.h's own comment on why this identity check
            // exists).
            friend class Z3Predicate;

            // Test/debug-only access hook, defined in the private (never-installed) Z3Internal.h
            // -- lets a core/tests/ verification harness reach the real z3::context (eg. to build
            // a z3::solver directly for a semantic-equivalence check), without opening up #impl to
            // any real (non-test) caller. See Z3Internal.h's own comment.
            friend Impl& getZ3ContextImplForTesting(Z3Context& ctx);
    };
}

#endif
