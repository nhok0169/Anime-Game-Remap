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

#include "AGRemapCore/tools/grapheme/GraphemeIterator.h"

#include <utf8proc.h>


namespace AGRemapCore {
    GraphemeIterator::GraphemeIterator(std::string_view text, size_t pos): m_text(text), m_pos(pos), m_state(0) {
        if (m_pos < m_text.size()) {
            findNext();
        }
    }

    std::string_view GraphemeIterator::operator*() const {
        return m_text.substr(m_pos, m_next - m_pos);
    }

    GraphemeIterator& GraphemeIterator::operator++() {
        m_pos = m_next;

        if (m_pos < m_text.size()) {
            findNext();
        }

        return *this;
    }

    bool GraphemeIterator::operator==(const GraphemeIterator& other) const {
        return m_pos == other.m_pos;
    }

    bool GraphemeIterator::operator!=(const GraphemeIterator& other) const {
        return !(*this == other);
    }

    void GraphemeIterator::findNext() {
        utf8proc_int32_t prev_cp = 0;
        utf8proc_int32_t curr_cp = 0;

        size_t offset = m_pos;

        utf8proc_ssize_t len =
            utf8proc_iterate(
                reinterpret_cast<const utf8proc_uint8_t*>(m_text.data()) + offset,
                m_text.size() - offset,
                &prev_cp);

        // A byte that is not valid UTF-8 (utf8proc reports a negative length for it) becomes a
        // 1-byte grapheme of its own. Adding the negative length to 'offset' instead, as this used
        // to, walked the iterator backwards over malformed input.
        if (len <= 0) {
            m_next = offset + 1;
            return;
        }

        offset += len;

        while (offset < m_text.size()) {
            len = utf8proc_iterate(
                reinterpret_cast<const utf8proc_uint8_t*>(m_text.data()) + offset,
                m_text.size() - offset,
                &curr_cp);

            // Same rule as above: a malformed byte always starts a new grapheme.
            if (len <= 0)
                break;

            if (utf8proc_grapheme_break_stateful(prev_cp, curr_cp, &m_state))
                break;

            prev_cp = curr_cp;
            offset += len;
        }

        m_next = offset;
    }
}
