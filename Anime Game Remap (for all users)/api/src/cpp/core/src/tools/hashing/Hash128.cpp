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

#include "AGRemapCore/tools/hashing/Hash128.h"

#include <xxhash.h>


namespace AGRemapCore {
    Hash128::Hash128(std::uint64_t high, std::uint64_t low) noexcept {
        for (std::size_t i = 0; i < 8; i++) {
            bytes_[i] = static_cast<std::uint8_t>(high >> (8 * (7 - i)));
            bytes_[8 + i] = static_cast<std::uint8_t>(low >> (8 * (7 - i)));
        }
    }

    Hash128 Hash128::hash(const void *data, std::size_t size) noexcept {
        XXH128_hash_t result = XXH3_128bits(data, size);
        return Hash128(result.high64, result.low64);
    }

    Hash128 Hash128::hash(std::string_view str) noexcept {
        return Hash128::hash(str.data(), str.size());
    }

    std::uint64_t Hash128::getHigh() const noexcept {
        std::uint64_t value = 0;
        for (std::size_t i = 0; i < 8; i++) {
            value = (value << 8) | bytes_[i];
        }

        return value;
    }

    std::uint64_t Hash128::getLow() const noexcept {
        std::uint64_t value = 0;
        for (std::size_t i = 8; i < 16; i++) {
            value = (value << 8) | bytes_[i];
        }

        return value;
    }
}
