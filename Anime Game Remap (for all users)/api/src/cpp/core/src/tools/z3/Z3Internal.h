#ifndef AGRemapCore_Z3Internal_H
#define AGRemapCore_Z3Internal_H

// -----------------------------------------------------------------------------
// Private, uninstalled header -- lives under core/src (not core/include), so it never ships as
// part of AGRemapCore's public SDK and is never on a consumer's include path. This is the ONE
// place where AGRemapCore::Z3Context::Impl / AGRemapCore::Z3Predicate::Impl are actually defined
// in terms of the real z3++.h types (z3::context / z3::expr) -- every public header
// (Z3Context.h/Z3Predicate.h/IfPredZ3Generator.h) only ever forward-declares the opaque Impl
// class, so #include-ing them never pulls in z3++.h.
//
// Include this only from a .cpp that itself already needs z3++.h directly (currently:
// Z3Context.cpp, Z3Predicate.cpp, IfPredZ3Generator.cpp, Z3IfPredGenerator.cpp, and the
// core/tests/ verification harnesses for both generators).
// -----------------------------------------------------------------------------

#include <memory>
#include <utility>

#include <z3++.h>

#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {

    // 'ctx' is a std::shared_ptr, not a plain z3::context, for a real (not hypothetical)
    // lifetime reason: a z3::expr (ast/object) only ever holds a raw, non-owning z3::context*
    // (Z3's own documented contract is that the context must outlive every expr built from it --
    // there's no refcounting on the C++-API side at all). A Z3Context living on the C++ stack
    // naturally outlives everything built from it (declared first, destroyed last), so this
    // never surfaced in this class's own standalone core/tests/ harnesses -- but nothing enforced
    // it, and Python's GC/interpreter-shutdown teardown order is not stack-scoped: a real,
    // reproducible access violation (0xC0000005) was hit once IfPredPart got a pybind11 binding
    // and real Python code held a Z3Context and several Z3Predicates built from it as ordinary,
    // independently-collectible objects -- whichever got garbage-collected/torn-down first, the
    // other's z3::expr destructor (or any subsequent use) dereferences a dangling z3::context*.
    // Sharing ownership here (Z3Predicate::Impl below holds the same shared_ptr) makes the
    // context's lifetime track "still referenced by at least one live Z3Predicate", regardless of
    // destruction order, instead of relying on the caller to keep it alive by convention.
    class Z3Context::Impl {
        public:
            explicit Impl(z3::context ctx): ctx(std::make_shared<z3::context>(std::move(ctx))) {}

            std::shared_ptr<z3::context> ctx;
    };

    class Z3Predicate::Impl {
        public:
            // 'ctxKeepAlive' -- see Z3Context::Impl's own comment above for why this exists: it's
            // never read directly, only held, so the owning Z3Context (and the real z3::context
            // 'predicate' points into) can never be destroyed while this Z3Predicate still is.
            Impl(z3::expr predicate, std::shared_ptr<z3::context> ctxKeepAlive):
                predicate(std::move(predicate)), ctxKeepAlive(std::move(ctxKeepAlive)) {}

            // Always a boolean-sorted expression -- IfPredZ3Generator::generate coerces the
            // top-level result to bool before ever wrapping it in a Z3Predicate.
            z3::expr predicate;

            std::shared_ptr<z3::context> ctxKeepAlive;
    };

    // The one function with real (friended) access to Z3Predicate's private constructor from
    // outside its own two real callers -- see the friend declaration's own comment in
    // Z3Predicate.h. Only ever called from core/tests/ verification code, never from real
    // AGRemapCore logic.
    inline Z3Predicate makeZ3PredicateForTesting(std::unique_ptr<Z3Predicate::Impl> impl) {
        return Z3Predicate(std::move(impl));
    }

    // Test-only access hooks -- see each one's own friend declaration's comment in
    // Z3Context.h/Z3Predicate.h. Only ever called from core/tests/ verification code.
    inline Z3Context::Impl& getZ3ContextImplForTesting(Z3Context& ctx) {
        return ctx.impl();
    }

    inline const Z3Predicate::Impl& getZ3PredicateImplForTesting(const Z3Predicate& predicate) {
        return predicate.impl();
    }
}

#endif
