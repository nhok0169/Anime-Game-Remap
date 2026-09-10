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
