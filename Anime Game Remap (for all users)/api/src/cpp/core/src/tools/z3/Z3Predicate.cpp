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

#include "AGRemapCore/tools/z3/Z3Predicate.h"

#include "tools/z3/Z3Internal.h"


namespace AGRemapCore {

    Z3Predicate::Z3Predicate(std::unique_ptr<Impl> impl): impl_(std::move(impl)) {}

    Z3Predicate::~Z3Predicate() = default;

    Z3Predicate::Z3Predicate(const Z3Predicate& other): impl_(std::make_unique<Impl>(*other.impl_)) {}

    Z3Predicate& Z3Predicate::operator=(const Z3Predicate& other) {
        if (this != &other) {
            impl_ = std::make_unique<Impl>(*other.impl_);
        }

        return *this;
    }

    Z3Predicate::Z3Predicate(Z3Predicate&&) noexcept = default;
    Z3Predicate& Z3Predicate::operator=(Z3Predicate&&) noexcept = default;

    std::string Z3Predicate::toString() const {
        return impl_->predicate.to_string();
    }

    Z3Predicate Z3Predicate::operator&(const Z3Predicate& other) const {
        return Z3Predicate(std::make_unique<Impl>(impl_->predicate && other.impl_->predicate, impl_->ctxKeepAlive));
    }

    Z3Predicate Z3Predicate::operator|(const Z3Predicate& other) const {
        return Z3Predicate(std::make_unique<Impl>(impl_->predicate || other.impl_->predicate, impl_->ctxKeepAlive));
    }

    Z3Predicate Z3Predicate::operator!() const {
        return Z3Predicate(std::make_unique<Impl>(!impl_->predicate, impl_->ctxKeepAlive));
    }

    Z3Predicate Z3Predicate::simplify() const {
        return Z3Predicate(std::make_unique<Impl>(impl_->predicate.simplify(), impl_->ctxKeepAlive));
    }


    Z3Predicate Z3Predicate::solverSimplify() const {
        z3::context& ctx = *impl_->ctxKeepAlive;

        // `propagate-values` first, and it is the one doing the work. An else-if chain accumulates
        // into `x != 0 AND x != 1 AND ... AND x == n`, and propagating the equality evaluates every
        // negation away in ONE rewriting pass with no solver at all -- 200 branches in well under a
        // second. `ctx-solver-simplify` ALONE does not do this: it drops one of the eleven negations
        // of a twelve-way chain and stops, and reaching the same answer by repeating it costs a pass
        // per branch. It runs second, on what is left, for the conjunctions propagation cannot reach.
        //
        // NOT `solve-eqs`, which looks like the obvious third and is disqualified: it ELIMINATES the
        // variable, answering `x == 11` with an empty goal. That is sound for asking whether the
        // thing is satisfiable and useless for writing a condition back out into a mod.
        z3::tactic tactic = z3::tactic(ctx, "propagate-values") & z3::tactic(ctx, "ctx-solver-simplify");

        z3::goal goal(ctx);
        goal.add(impl_->predicate);

        z3::apply_result result = tactic(goal);
        if (result.size() != 1) {
            // A tactic that split the goal, which neither of these does to a conjunction. Rather
            // than guess how to put the pieces back, keep what came in.
            return *this;
        }

        // An empty goal is everything discharged, which is `true` -- goal::as_expr says so itself,
        // and this is the shape a part with no enclosing condition arrives in.
        return Z3Predicate(std::make_unique<Impl>(result[0].as_expr(), impl_->ctxKeepAlive));
    }

    bool Z3Predicate::isSatisfiable() const {
        z3::solver solver(*impl_->ctxKeepAlive);
        solver.add(impl_->predicate);
        return solver.check() == z3::sat;
    }

    bool Z3Predicate::sameContext(const Z3Predicate& other) const {
        return impl_->ctxKeepAlive.get() == other.impl_->ctxKeepAlive.get();
    }

    bool Z3Predicate::belongsTo(const Z3Context& ctx) const {
        return impl_->ctxKeepAlive.get() == ctx.impl().ctx.get();
    }

    Z3Predicate Z3Predicate::trueValue(Z3Context& ctx) {
        Z3Context::Impl& ctxImpl = ctx.impl();
        return Z3Predicate(std::make_unique<Impl>(ctxImpl.ctx->bool_val(true), ctxImpl.ctx));
    }

    Z3Predicate Z3Predicate::falseValue(Z3Context& ctx) {
        Z3Context::Impl& ctxImpl = ctx.impl();
        return Z3Predicate(std::make_unique<Impl>(ctxImpl.ctx->bool_val(false), ctxImpl.ctx));
    }

    Z3Predicate::Impl& Z3Predicate::impl() {
        return *impl_;
    }

    const Z3Predicate::Impl& Z3Predicate::impl() const {
        return *impl_;
    }
}
