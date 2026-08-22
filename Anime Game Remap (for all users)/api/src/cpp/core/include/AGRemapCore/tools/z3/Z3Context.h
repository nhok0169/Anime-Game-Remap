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

     .. warning::
        **Known limitation, confirmed to be in `Z3`_'s own C++ API itself, not something this
        wrapper introduces or can fix**: every ``Z3Predicate`` built from a given ``Z3Context``
        keeps that context's underlying ``z3::context`` alive via a shared reference (so a
        ``Z3Context`` going out of scope/being garbage-collected in Python while
        ``Z3Predicate``s built from it are still alive is safe on its own). What is **not** safe:
        having *several independent* ``Z3Context`` instances whose own Python/C++ handles have
        already gone away (kept alive only via that shared reference from their predicates), then
        destroying those predicates in an order that *interleaves* across the different
        underlying contexts (eg. predicate-of-context-A, predicate-of-context-B, another
        predicate-of-context-A, ...) -- this reproducibly access-violates. Destroying them
        grouped-by-context (in any order, including a shuffled group order) is fine; destroying
        them while every context is still alive is fine; the crash requires the specific
        combination of "originating contexts already gone" + "cross-context-interleaved
        destruction order". Confirmed with a pure C++, zero-Python repro, so this is not a
        pybind11/GC-timing artifact. **Practical guidance**: keep the mainline usage pattern this
        codebase already follows -- one long-lived ``Z3Context`` shared by every predicate that
        needs to be comparable/combinable together (eg. one per .ini file, matching
        ``IniFile._z3Ctx``) -- rather than creating and discarding many short-lived contexts whose
        predicates might end up interleaved during teardown.
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
