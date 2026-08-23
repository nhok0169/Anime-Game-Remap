#include "AGRemapCore/model/buffers/BufFloat.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <variant>


namespace AGRemapCore {

    namespace {
        std::uint32_t bytesToU32(const ByteVec& src, bool bigEndian) {
            std::uint32_t acc = 0;
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

        std::uint16_t bytesToU16(const ByteVec& src, bool bigEndian) {
            std::uint16_t acc = 0;
            if (bigEndian) {
                acc = static_cast<std::uint16_t>((src[0] << 8) | src[1]);
            } else {
                acc = static_cast<std::uint16_t>((src[1] << 8) | src[0]);
            }
            return acc;
        }

        ByteVec u32ToBytes(std::uint32_t raw, bool bigEndian) {
            ByteVec result(4);
            for (std::size_t i = 0; i < 4; ++i) {
                std::uint8_t byte = static_cast<std::uint8_t>(raw & 0xFF);
                raw >>= 8;
                result[bigEndian ? (3 - i) : i] = byte;
            }
            return result;
        }

        ByteVec u16ToBytes(std::uint16_t raw, bool bigEndian) {
            ByteVec result(2);
            std::uint8_t low = static_cast<std::uint8_t>(raw & 0xFF);
            std::uint8_t high = static_cast<std::uint8_t>((raw >> 8) & 0xFF);
            if (bigEndian) {
                result[0] = high;
                result[1] = low;
            } else {
                result[0] = low;
                result[1] = high;
            }
            return result;
        }

        double toDouble(const BufValue& src) {
            return std::visit([](auto&& value) -> double { return static_cast<double>(value); }, src);
        }

        // IEEE 754 binary16 <-> binary32 conversion. This codebase's target platform (MSVC) has
        // no portable 'std::float16_t'/'_Float16' available, so this is done by hand rather than
        // relying on a compiler-specific half type.
        float halfToFloat(std::uint16_t half) {
            std::uint32_t sign = static_cast<std::uint32_t>(half & 0x8000) << 16;
            std::uint32_t exponent = (half >> 10) & 0x1F;
            std::uint32_t mantissa = half & 0x3FF;
            std::uint32_t bits;

            if (exponent == 0) {
                if (mantissa == 0) {
                    // +/- zero
                    bits = sign;
                } else {
                    // Subnormal half -> normalize into a normal float
                    exponent = 127 - 15 + 1;
                    while ((mantissa & 0x400) == 0) {
                        mantissa <<= 1;
                        --exponent;
                    }
                    mantissa &= 0x3FF;
                    bits = sign | (exponent << 23) | (mantissa << 13);
                }
            } else if (exponent == 0x1F) {
                // Inf/NaN
                bits = sign | 0x7F800000 | (mantissa << 13);
            } else {
                bits = sign | ((exponent - 15 + 127) << 23) | (mantissa << 13);
            }

            float result;
            std::memcpy(&result, &bits, sizeof(result));
            return result;
        }

        std::uint16_t floatToHalf(float value) {
            std::uint32_t bits;
            std::memcpy(&bits, &value, sizeof(bits));

            std::uint32_t sign = (bits >> 16) & 0x8000;
            std::int32_t exponent = static_cast<std::int32_t>((bits >> 23) & 0xFF) - 127 + 15;
            std::uint32_t mantissa = bits & 0x7FFFFF;

            if (exponent <= 0) {
                // Too small for a normal half -- flush to zero (denormal halves are not
                // round-tripped here, mirroring the precision this codebase's actual vertex
                // data -- blend weights/positions -- never approaches).
                return static_cast<std::uint16_t>(sign);
            }
            if (exponent >= 0x1F) {
                // Overflow -> infinity
                return static_cast<std::uint16_t>(sign | 0x7C00);
            }

            return static_cast<std::uint16_t>(sign | (static_cast<std::uint32_t>(exponent) << 10) | (mantissa >> 13));
        }
    }

    BufBaseFloat::BufBaseFloat(std::string name, std::size_t size, bool isBigEndian):
        BufDataType(std::move(name), size, isBigEndian) {}

    BufValue BufBaseFloat::decode(const ByteVec& src) const {
        std::uint32_t raw = bytesToU32(src, getIsBigEndian());
        float result;
        std::memcpy(&result, &raw, sizeof(result));
        return static_cast<double>(result);
    }

    ByteVec BufBaseFloat::encode(const BufValue& src) const {
        float value = static_cast<float>(toDouble(src));
        std::uint32_t raw;
        std::memcpy(&raw, &value, sizeof(raw));
        return u32ToBytes(raw, getIsBigEndian());
    }

    std::unique_ptr<BufDataType> BufBaseFloat::clone() const {
        return std::make_unique<BufBaseFloat>(*this);
    }

    BufFloat::BufFloat(bool isBigEndian): BufBaseFloat("Float32", 4, isBigEndian) {}

    std::unique_ptr<BufDataType> BufFloat::clone() const {
        return std::make_unique<BufFloat>(*this);
    }

    BufFloat16::BufFloat16(bool isBigEndian): BufBaseFloat("Float16", 2, isBigEndian) {}

    BufValue BufFloat16::decode(const ByteVec& src) const {
        std::uint16_t raw = bytesToU16(src, getIsBigEndian());
        return static_cast<double>(halfToFloat(raw));
    }

    ByteVec BufFloat16::encode(const BufValue& src) const {
        float value = static_cast<float>(toDouble(src));
        std::uint16_t raw = floatToHalf(value);
        return u16ToBytes(raw, getIsBigEndian());
    }

    std::unique_ptr<BufDataType> BufFloat16::clone() const {
        return std::make_unique<BufFloat16>(*this);
    }
}
