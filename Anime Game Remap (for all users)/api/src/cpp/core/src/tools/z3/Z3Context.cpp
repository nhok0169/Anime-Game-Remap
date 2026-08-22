#include "AGRemapCore/tools/z3/Z3Context.h"

#include "tools/z3/Z3Internal.h"


namespace AGRemapCore {

    Z3Context::Z3Context(): impl_(std::make_unique<Impl>(z3::context())) {}

    Z3Context::~Z3Context() = default;

    Z3Context::Z3Context(Z3Context&&) noexcept = default;
    Z3Context& Z3Context::operator=(Z3Context&&) noexcept = default;

    Z3Context::Impl& Z3Context::impl() {
        return *impl_;
    }

    const Z3Context::Impl& Z3Context::impl() const {
        return *impl_;
    }
}
