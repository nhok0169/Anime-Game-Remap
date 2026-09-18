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

#include "AGRemapCore/tools/StringHash.h"


namespace AGRemapCore {
    std::size_t StringViewHash::operator()(std::string_view sv) const noexcept {
        return std::hash<std::string_view>{}(sv);
    }
    std::size_t StringViewHash::operator()(const std::string& str) const noexcept {
        return std::hash<std::string>{}(str);
    }
    std::size_t StringViewHash::operator()(const char* ptr) const noexcept {
        return std::hash<std::string_view>{}(ptr);
    }
}