#include "AGRemapCore/model/buffers/BufUnorm.h"

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
        double value = toDouble(src);
        unsigned long long result = static_cast<unsigned long long>(value * static_cast<double>(maxValue_));
        return BufBaseInt::encode(result);
    }

    std::unique_ptr<BufDataType> BufUnorm::clone() const {
        return std::make_unique<BufUnorm>(*this);
    }
}
