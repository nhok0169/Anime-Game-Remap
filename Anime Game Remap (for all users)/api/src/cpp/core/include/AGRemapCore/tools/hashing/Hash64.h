#ifndef AGRemapCore_Hash64_H
#define AGRemapCore_Hash64_H

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

     A deterministic 64-bit hash id, the short counterpart to :cpp:class:`Hash128`.

     Currently backed by `XXH3-64`_, but no XXH3 type/header ever appears in this class's public
     interface -- callers only ever see :cpp:class:`Hash64` itself, so the underlying hashing
     library can be swapped out later without breaking anything that consumes this class.
     @endrst
     */
    class Hash64 : public HashInt<Hash64, 8> {
        public:
            /**
             * @brief Constructs a hash with a value of 0
             */
            Hash64() noexcept = default;

            /**
             * @brief Constructs a hash from its raw 64-bit value
             *
             * @param value The raw 64-bit value of the hash
             */
            explicit Hash64(std::uint64_t value) noexcept;

            /**
             * @brief Deterministically hashes a buffer of bytes
             *
             * @param data Pointer to the start of the buffer to hash
             * @param size The number of bytes in the buffer
             *
             * @return The resultant hash
             */
            static Hash64 hash(const void *data, std::size_t size) noexcept;

            /**
             * @brief Deterministically hashes a string
             *
             * @param str The string to hash
             *
             * @return The resultant hash
             */
            static Hash64 hash(std::string_view str) noexcept;

            /**
             * @brief The raw 64-bit value of the hash
             */
            std::uint64_t getValue() const noexcept;
    };
}


namespace std {
    /**
     * @brief Specialization so that :cpp:class:`AGRemapCore::Hash64` can be used out of the box
     * as a key in unordered containers (eg. ``std::unordered_map``, ``std::unordered_set``)
     */
    template <>
    struct hash<AGRemapCore::Hash64> {
        std::size_t operator()(const AGRemapCore::Hash64 &value) const noexcept {
            return value.hashCode();
        }
    };
}

#endif
