#include "AGRemapCore/tools/StringTools.h"

#include <utf8proc.h>
#include <ranges>

#include "AGRemapCore/tools/grapheme/GraphemeRange.h"


namespace AGRemapCore {

    namespace {
        // Decodes the codepoint starting at 'txt[pos]'. Returns how many bytes it took, always at
        // least 1: a byte that is not valid UTF-8 is reported as a 1-byte codepoint of -1, so that
        // callers walking a string never stall or step backwards on malformed input (utf8proc's
        // own error codes are negative byte counts).
        size_t decodeCodepoint(std::string_view txt, size_t pos, utf8proc_int32_t* codepoint) {
            utf8proc_ssize_t consumed = utf8proc_iterate(
                reinterpret_cast<const utf8proc_uint8_t*>(txt.data()) + pos,
                static_cast<utf8proc_ssize_t>(txt.size() - pos), codepoint);

            if (consumed <= 0) {
                *codepoint = -1;
                return 1;
            }

            return static_cast<size_t>(consumed);
        }

        // Python's own definition of a whitespace codepoint (Py_UNICODE_ISSPACE): bidirectional
        // class WS/B/S, or general category Zs. This also covers every ASCII whitespace character
        // (space and \t are S/WS, \n \r and \x1c-\x1e are B, \v and \x1f are S, \f is WS), so no
        // separate ASCII fast path is needed for the result to be right.
        bool isSpaceCodepoint(utf8proc_int32_t codepoint) {
            if (codepoint < 0) {
                return false;
            }

            const utf8proc_property_t* property = utf8proc_get_property(codepoint);
            return property->bidi_class == UTF8PROC_BIDI_CLASS_WS
                || property->bidi_class == UTF8PROC_BIDI_CLASS_B
                || property->bidi_class == UTF8PROC_BIDI_CLASS_S
                || property->category == UTF8PROC_CATEGORY_ZS;
        }
    }

    std::int8_t StringTools::compareStrPtrs(const std::string *strPtr1, const std::string *strPtr2) {
        int res = strPtr1->compare(*strPtr2);
        return (res > 0) - (res < 0);
    }

    size_t StringTools::countGrapheme(std::string_view txt) {
        // Walks the same grapheme boundaries GraphemeIterator does, so this count always agrees
        // with a GraphemeRange loop over the same text -- the two used to go through different
        // utf8proc entry points (this one via utf8proc_map + UTF8PROC_CHARBOUND), which also made
        // this the only grapheme routine that threw on malformed UTF-8.
        size_t count = 0;
        for ([[maybe_unused]] std::string_view grapheme : GraphemeRange(txt)) {
            ++count;
        }

        return count;
    }

    bool StringTools::isSpace(std::string_view grapheme) {
        if (grapheme.empty()) {
            return false;
        }

        size_t pos = 0;
        while (pos < grapheme.size()) {
            utf8proc_int32_t codepoint = -1;
            pos += decodeCodepoint(grapheme, pos, &codepoint);

            if (!isSpaceCodepoint(codepoint)) {
                return false;
            }
        }

        return true;
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

    std::string_view StringTools::lstrip(std::string_view txt) {
        size_t start = 0;
        for (std::string_view grapheme : GraphemeRange(txt)) {
            if (!isSpace(grapheme)) {
                break;
            }

            start += grapheme.size();
        }

        return txt.substr(start);
    }

    std::string_view StringTools::rstrip(std::string_view txt) {
        // Grapheme boundaries can only be found walking forwards (the break rules are stateful),
        // so this walks the whole text once and remembers where the last non-whitespace grapheme
        // ended, rather than trying to step backwards from the end.
        size_t end = 0;
        size_t pos = 0;
        for (std::string_view grapheme : GraphemeRange(txt)) {
            pos += grapheme.size();
            if (!isSpace(grapheme)) {
                end = pos;
            }
        }

        return txt.substr(0, end);
    }

    std::string_view StringTools::strip(std::string_view txt) {
        return rstrip(lstrip(txt));
    }

    std::string StringTools::toLower(std::string_view txt) {
        std::string result;
        result.reserve(txt.size());

        size_t pos = 0;
        while (pos < txt.size()) {
            utf8proc_int32_t codepoint = -1;
            size_t consumed = decodeCodepoint(txt, pos, &codepoint);

            if (codepoint < 0) {
                // Not valid UTF-8 -- passed through untouched rather than dropped or replaced, so
                // the result still lines up byte-for-byte with 'txt' outside of cased letters.
                result.append(txt.substr(pos, consumed));
                pos += consumed;
                continue;
            }

            utf8proc_uint8_t buf[4];
            utf8proc_ssize_t written = utf8proc_encode_char(utf8proc_tolower(codepoint), buf);
            if (written > 0) {
                result.append(reinterpret_cast<const char*>(buf), static_cast<size_t>(written));
            } else {
                result.append(txt.substr(pos, consumed));
            }

            pos += consumed;
        }

        return result;
    }

    std::string_view StringTools::firstGraphemes(std::string_view txt, size_t count) {
        size_t end = 0;
        size_t taken = 0;
        for (std::string_view grapheme : GraphemeRange(txt)) {
            if (taken == count) {
                break;
            }

            end += grapheme.size();
            ++taken;
        }

        return txt.substr(0, end);
    }

    std::string_view StringTools::lastGraphemes(std::string_view txt, size_t count) {
        if (count == 0) {
            return txt.substr(txt.size());
        }

        // Same forwards-only constraint as rstrip: collect where every grapheme starts, then the
        // wanted suffix begins at the start of the 'count'-th grapheme from the end.
        std::vector<size_t> starts;
        size_t pos = 0;
        for (std::string_view grapheme : GraphemeRange(txt)) {
            starts.push_back(pos);
            pos += grapheme.size();
        }

        if (starts.size() <= count) {
            return txt;
        }

        return txt.substr(starts[starts.size() - count]);
    }

    bool StringTools::startsWith(std::string_view txt, std::string_view prefix) {
        return firstGraphemes(txt, countGrapheme(prefix)) == prefix;
    }

    bool StringTools::endsWith(std::string_view txt, std::string_view suffix) {
        return lastGraphemes(txt, countGrapheme(suffix)) == suffix;
    }

    bool StringTools::equalsIgnoreCase(std::string_view txt1, std::string_view txt2) {
        return toLower(txt1) == toLower(txt2);
    }

    bool StringTools::endsWithIgnoreCase(std::string_view txt, std::string_view suffix) {
        size_t suffixLen = countGrapheme(suffix);
        std::string_view tail = lastGraphemes(txt, suffixLen);
        return countGrapheme(tail) == suffixLen && equalsIgnoreCase(tail, suffix);
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
