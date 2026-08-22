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
