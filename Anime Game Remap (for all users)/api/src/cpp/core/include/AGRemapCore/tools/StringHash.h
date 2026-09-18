#ifndef AGRemapCore_StringHash_H
#define AGRemapCore_StringHash_H

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