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
        //
        // THIS IS NOW THE ONLY HALF CODEC IN THE CODEBASE (2026-09-29). It used to carry a note
        // telling you not to merge it with the pair of the same name in
        // data/IniFixData/WWMIFixer.cpp, because that one rounds HALF TO EVEN -- numpy's float16
        // cast, and so the prototype that is the WuWa fix's oracle -- where this one TRUNCATED
        // (`mantissa >> 13`). A difference of 104 in 1,508,336 halves, every one a moved UV.
        //
        // Two codecs for one format is not a fact about the format; it was a missing PARAMETER.
        // `BufFloat16::Rounding` is it, and the WWMIFixer pair is deleted. `Truncate` is the
        // default and is what this always did, so nothing the library already writes moves.
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

        std::uint16_t floatToHalfTruncate(float value) {
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

        // Round half to even, keeping subnormals and NaNs -- which is what makes decode() then
        // encode() an exact identity over all 65536 half bit patterns, and so what lets
        // BufFile::fix (which re-encodes every line) run over a buffer of halves without moving the
        // ones no filter touched.
        std::uint16_t floatToHalfNearestEven(float value) {
            std::uint32_t bits;
            std::memcpy(&bits, &value, sizeof(bits));

            const std::uint32_t sign = (bits >> 16) & 0x8000;
            const std::int32_t rawExponent = static_cast<std::int32_t>((bits >> 23) & 0xFF);
            const std::uint32_t mantissa = bits & 0x7FFFFF;

            // NaN stays NaN rather than becoming infinity: keep the payload's high bits, and make
            // sure the result is still a NaN when every bit that survived was zero.
            if (rawExponent == 0xFF) {
                if (mantissa != 0) {
                    std::uint32_t kept = mantissa >> 13;
                    if (kept == 0) {
                        kept = 1;
                    }

                    return static_cast<std::uint16_t>(sign | 0x7C00 | kept);
                }

                return static_cast<std::uint16_t>(sign | 0x7C00);
            }

            const std::int32_t exponent = rawExponent - 127 + 15;

            if (exponent >= 0x1F) {
                return static_cast<std::uint16_t>(sign | 0x7C00);            // overflow -> infinity
            }

            if (exponent > 0) {
                // A normal half. Round the 13 dropped bits half to even; a carry out of the
                // mantissa lands in the exponent by construction, which is why the addition is
                // done on the assembled value rather than on the mantissa alone.
                std::uint32_t assembled = (static_cast<std::uint32_t>(exponent) << 10) | (mantissa >> 13);
                const std::uint32_t dropped = mantissa & 0x1FFF;
                if (dropped > 0x1000 || (dropped == 0x1000 && (assembled & 1) != 0)) {
                    ++assembled;                                             // may carry into the exponent
                }

                if (assembled >= 0x7C00) {
                    return static_cast<std::uint16_t>(sign | 0x7C00);        // rounded up to infinity
                }

                return static_cast<std::uint16_t>(sign | assembled);
            }

            // Subnormal territory. Too small even for the smallest subnormal -> zero; otherwise
            // shift the implicit leading 1 in and round the dropped bits the same way. A carry out
            // of the subnormal mantissa produces the smallest NORMAL half, which is correct.
            if (exponent < -10) {
                return static_cast<std::uint16_t>(sign);
            }

            const std::uint32_t withImplicit = mantissa | 0x800000;
            const std::uint32_t shift = static_cast<std::uint32_t>(14 - exponent);
            std::uint32_t assembled = withImplicit >> shift;
            const std::uint32_t dropped = withImplicit & ((1u << shift) - 1);
            const std::uint32_t half = 1u << (shift - 1);
            if (dropped > half || (dropped == half && (assembled & 1) != 0)) {
                ++assembled;
            }

            return static_cast<std::uint16_t>(sign | assembled);
        }

        std::uint16_t floatToHalf(float value) {
            return floatToHalfTruncate(value);
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

    BufFloat16::BufFloat16(bool isBigEndian, Rounding rounding):
        BufBaseFloat("Float16", 2, isBigEndian), rounding_(rounding) {}

    BufFloat16::Rounding BufFloat16::getRounding() const {
        return rounding_;
    }

    BufValue BufFloat16::decode(const ByteVec& src) const {
        std::uint16_t raw = bytesToU16(src, getIsBigEndian());
        return static_cast<double>(halfToFloat(raw));
    }

    ByteVec BufFloat16::encode(const BufValue& src) const {
        float value = static_cast<float>(toDouble(src));
        std::uint16_t raw = (rounding_ == Rounding::NearestEven) ? floatToHalfNearestEven(value)
                                                                 : floatToHalfTruncate(value);
        return u16ToBytes(raw, getIsBigEndian());
    }

    std::unique_ptr<BufDataType> BufFloat16::clone() const {
        return std::make_unique<BufFloat16>(*this);
    }
}
