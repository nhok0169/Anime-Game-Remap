#ifndef AGRemapCore_GraphemeRange_H
#define AGRemapCore_GraphemeRange_H

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


namespace AGRemapCore {

    /**
     @brief
     @rst
     used to iterate over a range of `graphemes`_ within a UTF-8 string
     @endrst
     */
    class GraphemeRange {
        public:

            /**
             * @brief Constructs the range iterator
             *
             * @param text The text to iterate over
             */
            explicit GraphemeRange(std::string_view text);

            /**
             * @brief
             @rst
             Retrieves the iterator to the first `grapheme`_ of the string
             @endrst
             *
             * @return
             @rst
             The iterator pointing to the first `grapheme`_ of the string
             @endrst
             */
            GraphemeIterator begin() const;

            /**
             * @brief Retrieves the end of the iterator
             *
             * @return The iterator pointing to the end of the string
             */
            GraphemeIterator end() const;

        private:
            std::string_view m_text;
    };
}

#endif