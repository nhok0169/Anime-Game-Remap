#include "AGRemapCore/tools/hashing/HashInt.h"


namespace AGRemapCore {
    template <typename Derived, std::size_t ByteSize>
    std::string HashInt<Derived, ByteSize>::toHexString() const {
        static const char hexDigits[] = "0123456789abcdef";

        std::string result;
        result.reserve(ByteSize * 2);

        for (std::uint8_t byte : bytes_) {
            result += hexDigits[(byte >> 4) & 0xF];
            result += hexDigits[byte & 0xF];
        }

        return result;
    }

    template <typename Derived, std::size_t ByteSize>
    std::string HashInt<Derived, ByteSize>::toBase64() const {
        return detail::encodeBase64(bytes_.data(), bytes_.size());
    }

    template <typename Derived, std::size_t ByteSize>
    bool HashInt<Derived, ByteSize>::operator==(const HashInt &other) const noexcept {
        return bytes_ == other.bytes_;
    }

    template <typename Derived, std::size_t ByteSize>
    bool HashInt<Derived, ByteSize>::operator!=(const HashInt &other) const noexcept {
        return !(*this == other);
    }

    template <typename Derived, std::size_t ByteSize>
    bool HashInt<Derived, ByteSize>::operator<(const HashInt &other) const noexcept {
        return bytes_ < other.bytes_;
    }

    template <typename Derived, std::size_t ByteSize>
    std::size_t HashInt<Derived, ByteSize>::hashCode() const noexcept {
        // FNV-1a
        std::size_t h = 1469598103934665603ull;
        for (std::uint8_t byte : bytes_) {
            h ^= byte;
            h *= 1099511628211ull;
        }

        return h;
    }
}
