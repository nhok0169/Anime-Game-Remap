#ifndef AGRemapCore_StringHash_H
#define AGRemapCore_StringHash_H

#include <string>
#include <string_view>


namespace AGRemapCore {

    /**
     * @brief Hasing callable that supports string_view
     */
    struct StringViewHash {
        using is_transparent = void;

        /**
         * @brief Hashes a string_view
         */
        std::size_t operator()(std::string_view sv) const noexcept;

        /**
         * @brief Hashes a constant reference to a string
         */
        std::size_t operator()(const std::string& str) const noexcept;

        /**
         * @brief Hashes a char pointer that acts as a string
         */
        std::size_t operator()(const char* ptr) const noexcept;
    };
}

#endif