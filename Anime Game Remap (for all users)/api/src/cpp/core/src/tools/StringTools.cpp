#include "AGRemapCore/tools/StringTools.h"

#include <cctype>
#include <stdexcept>
#include <utf8proc.h>
#include <ranges>

namespace AGRemapCore {
    std::int8_t StringTools::compareStrPtrs(const std::string *strPtr1, const std::string *strPtr2) {
        int res = strPtr1->compare(*strPtr2);
        return (res > 0) - (res < 0);
    }

    size_t StringTools::countGrapheme(const std::string &txt) {
        utf8proc_uint8_t* mapped_ptr = nullptr;
        
        // Map the string to add 0xFF bytes at the start of each grapheme cluster
        // UTF8PROC_STABLE ensures normalization isn't aggressively changing characters, 
        // but applying it is usually needed to correctly calculate graphemes.
        // Pass 0 as flags if you do not need normalization or property checks.
        size_t mapped_len = utf8proc_map(
            reinterpret_cast<const utf8proc_uint8_t*>(txt.data()), 
            txt.size(), 
            &mapped_ptr, 
            static_cast<utf8proc_option_t>(UTF8PROC_STABLE | UTF8PROC_CHARBOUND)
        );

        if (mapped_len < 0) {
            throw std::runtime_error("utf8proc mapping failed: " + std::string(utf8proc_errmsg(mapped_len)));
        }

        // Every grapheme is prefixed with a 0xFF byte.
        size_t grapheme_count = 0;
        for (size_t i = 0; i < mapped_len; ++i) {
            if (mapped_ptr[i] == 0xFF) {
                grapheme_count++;
            }
        }

        // Free the memory allocated by utf8proc_map
        free(mapped_ptr);
        return grapheme_count;
    }

    std::vector<std::string_view> StringTools::splitlines(std::string_view txt) {
        std::vector<std::string_view> lines;
        
        for (auto&& chunk : txt | std::views::split('\n')) {
            std::string_view sv(chunk.begin(), chunk.end());
            
            // Strip trailing \r to fully match Python's splitlines
            if (!sv.empty() && sv.back() == '\r') {
                sv.remove_suffix(1);
            }
            
            lines.push_back(sv);
        }

        // A trailing '\n' (or "\r\n") terminates the last line rather than starting a new,
        // empty one -- e.g. Python's "abc\n".splitlines() == ["abc"], not ["abc", ""].
        // views::split('\n') still yields that trailing empty chunk (post-\r-stripping) since
        // it only knows about the '\n' separator itself, not line-terminator semantics.
        // Verified empirically against CPython's str.splitlines() for the empty string, a bare
        // "\n", "\r\n", and consecutive newlines (each preserves its own empty line) before this
        // fix went in.
        if (!txt.empty() && txt.back() == '\n' && !lines.empty()) {
            lines.pop_back();
        }

        return lines;
    }

    std::string_view StringTools::strip(std::string_view txt) {
        size_t start = 0;
        size_t end = txt.size();

        while (start < end && std::isspace(static_cast<unsigned char>(txt[start]))) {
            start++;
        }

        while (end > start && std::isspace(static_cast<unsigned char>(txt[end - 1]))) {
            end--;
        }

        return txt.substr(start, end - start);
    }

    void StringTools::eraseAll(std::string& txt, std::string_view target) {
        if (target.empty()) {
            return;
        }

        size_t pos = 0;
        while ((pos = txt.find(target, pos)) != std::string::npos) {
            txt.erase(pos, target.size());
        }
    }
}
