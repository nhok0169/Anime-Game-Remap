#include "AGRemapCore/model/buffers/BufInt.h"

#include <memory>
#include <utility>
#include <variant>


namespace AGRemapCore {

    namespace {
        // Reassembles 'src' (already validated to be <= 8 bytes by BufDataType) into a raw,
        // unsigned 64-bit accumulator, honouring 'bigEndian' -- the shared first step for both a
        // signed and an unsigned decode.
        unsigned long long bytesToRaw(const ByteVec& src, bool bigEndian) {
            unsigned long long acc = 0;

            if (bigEndian) {
                for (std::uint8_t byte : src) {
                    acc = (acc << 8) | byte;
                }
            } else {
                for (std::size_t i = src.size(); i-- > 0; ) {
                    acc = (acc << 8) | src[i];
                }
            }

            return acc;
        }

        // Sign-extends the low 'byteCount' bytes of 'raw' up to the full 64-bit width, matching
        // Python's int.from_bytes(..., signed=True).
        long long signExtend(unsigned long long raw, std::size_t byteCount) {
            std::size_t bits = byteCount * 8;
            if (bits >= 64) {
                return static_cast<long long>(raw);
            }

            unsigned long long signBit = 1ULL << (bits - 1);
            if (raw & signBit) {
                raw |= (~0ULL << bits);
            }

            return static_cast<long long>(raw);
        }

        // Coerces any BufValue alternative to a raw, unsigned 64-bit pattern -- used by encode()
        // so a filter can hand back whichever numeric alternative is convenient (eg. a plain
        // 'long long' even for a field that was originally decoded as unsigned), matching how
        // Python callers never had to think about int signedness in the first place.
        unsigned long long toRawBits(const BufValue& src) {
            return std::visit([](auto&& value) -> unsigned long long {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, double>) {
                    return static_cast<unsigned long long>(static_cast<long long>(value));
                } else {
                    return static_cast<unsigned long long>(value);
                }
            }, src);
        }
    }

    BufBaseInt::BufBaseInt(std::string name, std::size_t size, bool isBigEndian, bool isSigned):
        BufDataType(std::move(name), size, isBigEndian), isSigned_(isSigned) {}

    bool BufBaseInt::getIsSigned() const {
        return isSigned_;
    }

    BufValue BufBaseInt::decode(const ByteVec& src) const {
        unsigned long long raw = bytesToRaw(src, getIsBigEndian());

        if (isSigned_) {
            return signExtend(raw, src.size());
        }
        return raw;
    }

    ByteVec BufBaseInt::encode(const BufValue& src) const {
        unsigned long long raw = toRawBits(src);
        std::size_t size = getSize();
        bool bigEndian = getIsBigEndian();

        ByteVec result(size);
        for (std::size_t i = 0; i < size; ++i) {
            std::uint8_t byte = static_cast<std::uint8_t>(raw & 0xFF);
            raw >>= 8;
            result[bigEndian ? (size - 1 - i) : i] = byte;
        }

        return result;
    }

    std::unique_ptr<BufDataType> BufBaseInt::clone() const {
        return std::make_unique<BufBaseInt>(*this);
    }

    BufSignedInt::BufSignedInt(std::string name, std::size_t size, bool isBigEndian):
        BufBaseInt(std::move(name), size, isBigEndian, true) {}

    std::unique_ptr<BufDataType> BufSignedInt::clone() const {
        return std::make_unique<BufSignedInt>(*this);
    }

    BufUnSignedInt::BufUnSignedInt(std::string name, std::size_t size, bool isBigEndian):
        BufBaseInt(std::move(name), size, isBigEndian, false) {}

    std::unique_ptr<BufDataType> BufUnSignedInt::clone() const {
        return std::make_unique<BufUnSignedInt>(*this);
    }
}
