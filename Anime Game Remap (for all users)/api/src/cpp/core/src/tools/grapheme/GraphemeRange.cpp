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