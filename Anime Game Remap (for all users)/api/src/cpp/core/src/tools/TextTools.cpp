#include "AGRemapCore/tools/TextTools.h"

#include <stdexcept>
#include <vector>
#include <utf8proc.h>


namespace AGRemapCore {
    std::string TextTools::capitalize(std::string_view txt) {
        if (txt.empty()) {
            return std::string(txt);
        }

        utf8proc_int32_t codepoint = -1;
        utf8proc_ssize_t consumed = utf8proc_iterate(
            reinterpret_cast<const utf8proc_uint8_t*>(txt.data()), static_cast<utf8proc_ssize_t>(txt.size()), &codepoint);
        if (consumed <= 0) {
            throw std::runtime_error("TextTools::capitalize: invalid UTF-8 input");
        }

        utf8proc_int32_t upper = utf8proc_toupper(codepoint);

        utf8proc_uint8_t buf[4];
        utf8proc_ssize_t written = utf8proc_encode_char(upper, buf);

        std::string result;
        result.reserve(txt.size());
        result.append(reinterpret_cast<const char*>(buf), static_cast<size_t>(written));
        result.append(txt.substr(static_cast<size_t>(consumed)));
        return result;
    }

    std::string TextTools::reverse(std::string_view txt) {
        // Decode to codepoints first, then re-encode in reverse order -- reversing the raw UTF-8
        // bytes directly would corrupt every multi-byte character. This still matches Python's
        // txt[::-1] exactly (codepoint-level reversal), not a "safer" grapheme-level reversal --
        // see the header's own note on this.
        std::vector<utf8proc_int32_t> codepoints;
        codepoints.reserve(txt.size());

        size_t pos = 0;
        while (pos < txt.size()) {
            utf8proc_int32_t codepoint = -1;
            utf8proc_ssize_t consumed = utf8proc_iterate(
                reinterpret_cast<const utf8proc_uint8_t*>(txt.data()) + pos,
                static_cast<utf8proc_ssize_t>(txt.size() - pos), &codepoint);
            if (consumed <= 0) {
                throw std::runtime_error("TextTools::reverse: invalid UTF-8 input");
            }

            codepoints.push_back(codepoint);
            pos += static_cast<size_t>(consumed);
        }

        std::string result;
        result.reserve(txt.size());
        for (auto it = codepoints.rbegin(); it != codepoints.rend(); ++it) {
            utf8proc_uint8_t buf[4];
            utf8proc_ssize_t written = utf8proc_encode_char(*it, buf);
            if (written > 0) {
                result.append(reinterpret_cast<const char*>(buf), static_cast<size_t>(written));
            }
        }

        return result;
    }
}
