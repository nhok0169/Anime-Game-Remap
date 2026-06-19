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