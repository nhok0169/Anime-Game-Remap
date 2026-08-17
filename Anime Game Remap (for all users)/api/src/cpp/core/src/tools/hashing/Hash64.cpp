#include "AGRemapCore/tools/hashing/Hash64.h"

#include <xxhash.h>


namespace AGRemapCore {
    Hash64::Hash64(std::uint64_t value) noexcept {
        for (std::size_t i = 0; i < bytes_.size(); i++) {
            bytes_[i] = static_cast<std::uint8_t>(value >> (8 * (bytes_.size() - 1 - i)));
        }
    }

    Hash64 Hash64::hash(const void *data, std::size_t size) noexcept {
        return Hash64(XXH3_64bits(data, size));
    }

    Hash64 Hash64::hash(std::string_view str) noexcept {
        return Hash64::hash(str.data(), str.size());
    }

    std::uint64_t Hash64::getValue() const noexcept {
        std::uint64_t value = 0;
        for (std::uint8_t byte : bytes_) {
            value = (value << 8) | byte;
        }

        return value;
    }
}
