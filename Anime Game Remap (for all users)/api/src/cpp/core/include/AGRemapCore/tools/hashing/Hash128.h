#ifndef AGRemapCore_Hash128_H
#define AGRemapCore_Hash128_H

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <functional>

#include "AGRemapCore/tools/hashing/HashInt.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`HashInt`.

     A deterministic 128-bit hash id, the long counterpart to :cpp:class:`Hash64`.

     Currently backed by `XXH3-128`_, but no XXH3 type/header ever appears in this class's
     public interface -- callers only ever see :cpp:class:`Hash128` itself, so the underlying
     hashing library can be swapped out later without breaking anything that consumes this
     class.
     @endrst
     */
    class Hash128 : public HashInt<Hash128, 16> {
        public:
            /**
             * @brief Constructs a hash with both halves set to 0
             */
            Hash128() noexcept = default;

            /**
             * @brief Constructs a hash from its 2 64-bit halves
             *
             * @param high The high 64 bits of the hash
             * @param low The low 64 bits of the hash
             */
            Hash128(std::uint64_t high, std::uint64_t low) noexcept;

            /**
             * @brief Deterministically hashes a buffer of bytes
             *
             * @param data Pointer to the start of the buffer to hash
             * @param size The number of bytes in the buffer
             *
             * @return The resultant hash
             */
            static Hash128 hash(const void *data, std::size_t size) noexcept;

            /**
             * @brief Deterministically hashes a string
             *
             * @param str The string to hash
             *
             * @return The resultant hash
             */
            static Hash128 hash(std::string_view str) noexcept;

            /**
             * @brief The high 64 bits of the hash
             */
            std::uint64_t getHigh() const noexcept;

            /**
             * @brief The low 64 bits of the hash
             */
            std::uint64_t getLow() const noexcept;
    };
}


namespace std {
    /**
     * @brief Specialization so that :cpp:class:`AGRemapCore::Hash128` can be used out of the box
     * as a key in unordered containers (eg. ``std::unordered_map``, ``std::unordered_set``)
     */
    template <>
    struct hash<AGRemapCore::Hash128> {
        std::size_t operator()(const AGRemapCore::Hash128 &value) const noexcept {
            return value.hashCode();
        }
    };
}

#endif
