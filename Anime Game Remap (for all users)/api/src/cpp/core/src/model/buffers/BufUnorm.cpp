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

#include "AGRemapCore/model/buffers/BufUnorm.h"

#include <cmath>
#include <memory>
#include <utility>
#include <variant>


namespace AGRemapCore {

    namespace {
        unsigned long long computeMaxValue(std::size_t size) {
            // pow(2, size * 8) - 1, computed exactly for size <= 8 (BufDataType's constructor
            // already rejects size > 8) instead of via floating point pow().
            if (size >= 8) {
                return ~0ULL;
            }
            return (1ULL << (size * 8)) - 1;
        }

        double toDouble(const BufValue& src) {
            return std::visit([](auto&& value) -> double { return static_cast<double>(value); }, src);
        }
    }

    BufUnorm::BufUnorm(std::string name, std::size_t size, bool isBigEndian):
        BufBaseInt(std::move(name), size, isBigEndian, false), maxValue_(computeMaxValue(size)) {}

    BufValue BufUnorm::decode(const ByteVec& src) const {
        BufValue numerator = BufBaseInt::decode(src);
        return static_cast<double>(std::get<unsigned long long>(numerator)) / static_cast<double>(maxValue_);
    }

    ByteVec BufUnorm::encode(const BufValue& src) const {
        // Rounded to the nearest step, as D3D converts a float to UNORM. Truncating lost a value
        // that arrives as dump TEXT: 3dmigoto prints 126/255 as 0.494117677, which times 255 is
        // 125.99999..., so a frame dump read through VbFile::readDumpStr wrote a vertex colour
        // of 125 where the game's buffer holds 126 (one of the 256 UNORM8 values; decode ->
        // encode was exact either way).
        double scaled = std::floor(toDouble(src) * static_cast<double>(maxValue_) + 0.5);
        double maxValue = static_cast<double>(maxValue_);
        unsigned long long result = (scaled <= 0) ? 0ULL : ((scaled >= maxValue) ? maxValue_ : static_cast<unsigned long long>(scaled));
        return BufBaseInt::encode(result);
    }

    std::unique_ptr<BufDataType> BufUnorm::clone() const {
        return std::make_unique<BufUnorm>(*this);
    }
}
