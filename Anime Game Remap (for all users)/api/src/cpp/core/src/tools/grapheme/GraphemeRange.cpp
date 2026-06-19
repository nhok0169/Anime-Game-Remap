#include "AGRemapCore/tools/grapheme/GraphemeRange.h"


namespace AGRemapCore {
    GraphemeRange::GraphemeRange(std::string_view text): m_text(text) {

    }

    GraphemeIterator GraphemeRange::begin() const {
        return GraphemeIterator(m_text, 0);
    }

    GraphemeIterator GraphemeRange::end() const {
        return GraphemeIterator(m_text, m_text.size());
    }
}