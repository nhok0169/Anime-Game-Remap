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


// BufFloat16::Rounding -- the property the WuWa texcoord fix depends on.
//
// `BufFile::fix` re-encodes EVERY line, including the ones no filter touched, so running it over a
// buffer of halves is only safe if decode-then-encode cannot change a value. Under
// Rounding::Truncate it demonstrably can, which is why `buildTexcoordCopy` used to patch the bytes
// by hand and why a second half codec lived in WWMIFixer.cpp.
//
// This asserts the whole claim exhaustively rather than on samples: all 65536 half bit patterns.
//
//   cl /std:c++17 /EHsc /I <core>/include /I <core>/src BufFloat16_Rounding_test.cpp \
//      <core>/src/model/buffers/BufFloat.cpp <core>/src/model/buffers/BufDataType.cpp

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "AGRemapCore/model/buffers/BufFloat.h"

using namespace AGRemapCore;


namespace {

    int failures = 0;

    void check(bool condition, const char* what, std::uint16_t bits) {
        if (!condition) {
            ++failures;
            if (failures <= 20) {
                std::printf("  FAIL %s: half 0x%04X\n", what, static_cast<unsigned>(bits));
            }
        }
    }

    ByteVec bytesOf(std::uint16_t bits) {
        return ByteVec{static_cast<std::uint8_t>(bits & 0xFF), static_cast<std::uint8_t>(bits >> 8)};
    }

    std::uint16_t bitsOf(const ByteVec& bytes) {
        return static_cast<std::uint16_t>(bytes[0] | (static_cast<std::uint16_t>(bytes[1]) << 8));
    }

    bool isNaNHalf(std::uint16_t bits) {
        return ((bits >> 10) & 0x1F) == 0x1F && (bits & 0x3FF) != 0;
    }
}


int main() {
    std::printf("=== BufFloat16::Rounding ===\n");

    const BufFloat16 exact(false, BufFloat16::Rounding::NearestEven);
    const BufFloat16 truncating;                         // the default, unchanged

    // ---- 1. NearestEven round-trips every one of the 65536 half bit patterns ----------------
    for (std::uint32_t raw = 0; raw <= 0xFFFF; ++raw) {
        const auto bits = static_cast<std::uint16_t>(raw);
        const std::uint16_t back = bitsOf(exact.encode(exact.decode(bytesOf(bits))));

        if (isNaNHalf(bits)) {
            // A NaN must come back a NaN. Its payload is not required to survive a widening and
            // narrowing, but it must not turn into infinity, which is a NUMBER.
            check(isNaNHalf(back), "NaN became a non-NaN", bits);
            continue;
        }

        check(back == bits, "NearestEven is not an identity", bits);
    }
    std::printf("all 65536 half bit patterns round-trip under NearestEven\n");

    // ---- 2. the default is untouched, and is NOT an identity --------------------------------
    // Proving the second half matters as much as the first: it is what says the new mode is
    // actually doing something, rather than the two modes having quietly become the same.
    int truncateDiffers = 0;
    for (std::uint32_t raw = 0; raw <= 0xFFFF; ++raw) {
        const auto bits = static_cast<std::uint16_t>(raw);
        if (isNaNHalf(bits)) {
            continue;
        }

        if (bitsOf(truncating.encode(truncating.decode(bytesOf(bits)))) != bits) {
            ++truncateDiffers;
        }
    }

    check(truncateDiffers > 0, "Truncate has silently become exact -- the modes are not distinct", 0);
    std::printf("Truncate loses %d of 65536 patterns (subnormals flushed to zero), as it always has\n",
                truncateDiffers);

    // ---- 3. the rounding itself, on a value that lands exactly between two halves ------------
    // 1.0 + 2^-11 is the midpoint between 1.0 (mantissa 0, even) and the next half up (mantissa 1,
    // odd), so half-to-even must round DOWN to 1.0 while truncation also gives 1.0 -- and the
    // midpoint one step up must round UP to an even mantissa. Sampled rather than exhaustive
    // because the exhaustive test above cannot see rounding: it only ever feeds exact halves.
    {
        const double midDown = 1.0 + std::pow(2.0, -11);                  // between 1.0 and 1.0009765625
        check(bitsOf(exact.encode(BufValue(midDown))) == 0x3C00, "midpoint did not round to even (down)", 0);

        const double oneUp = 1.0009765625;                                 // mantissa 1, odd
        const double midUp = oneUp + std::pow(2.0, -11);                   // between mantissa 1 and 2
        check(bitsOf(exact.encode(BufValue(midUp))) == 0x3C02, "midpoint did not round to even (up)", 0);
    }
    std::printf("half-to-even holds at both midpoints\n");

    // ---- 4. a subnormal survives, where the default flushes it -------------------------------
    {
        const ByteVec smallest = bytesOf(0x0001);                          // the smallest subnormal half
        check(bitsOf(exact.encode(exact.decode(smallest))) == 0x0001, "subnormal not round-tripped", 0x0001);
        check(bitsOf(truncating.encode(truncating.decode(smallest))) == 0x0000,
              "Truncate no longer flushes a subnormal", 0x0001);
    }
    std::printf("subnormals: kept under NearestEven, flushed under Truncate\n");

    // ---- 5. clone() carries the mode ---------------------------------------------------------
    // Everything that builds a BufFile copies its elements, so a mode that did not survive clone()
    // would be silently ignored by every real caller while passing every test above.
    {
        const std::unique_ptr<BufDataType> copy = exact.clone();
        const auto* asHalf = dynamic_cast<const BufFloat16*>(copy.get());
        check(asHalf != nullptr, "clone() did not produce a BufFloat16", 0);
        if (asHalf != nullptr) {
            check(asHalf->getRounding() == BufFloat16::Rounding::NearestEven, "clone() lost the rounding mode", 0);
            check(bitsOf(asHalf->encode(asHalf->decode(bytesOf(0x0001)))) == 0x0001,
                  "the clone does not round-trip a subnormal", 0x0001);
        }
    }
    std::printf("clone() carries the mode\n");

    if (failures == 0) {
        std::printf("\nALL PASSED\n");
        return 0;
    }

    std::printf("\n%d FAILURE(S)\n", failures);
    return 1;
}
