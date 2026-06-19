#ifndef AGRemapCore_GraphemeIterator_H
#define AGRemapCore_GraphemeIterator_H

#include <string_view>
#include <cstdint>


namespace AGRemapCore {

    /**
     @brief
     @rst
     An iterator for `graphemes`_ within a UTF-8 string
     @endrst
     */
    class GraphemeIterator {
        public:
            /**
             * @brief Constructs the iterator
             *
             * @param text The source text to iterate over
             *
             * @param pos The starting index to iterate from
             * 
             * **Default**: `0`
             */
            GraphemeIterator(std::string_view text, size_t pos = 0);

            /**
             * @brief Retrieves the string view of the current grapheme
             *
             * @return The current grapheme
             */
            std::string_view operator*() const;

            /**
             * @brief  Retrieves the next grapheme for the iterator
             *
             * @return The iterator moved to the next grapheme
             */
            GraphemeIterator& operator++();

            /**
             * @brief Determine whether 2 iterators are at the same character index position
             *
             * @warning Both iterators could compare different strings but both be at the same character index position
             *
             * @param other The other iterator to compare
             *
             * @return Whether both iterators are at the same character index position
             */
            bool operator==(const GraphemeIterator& other) const;
            bool operator!=(const GraphemeIterator& other) const;

        private:
            std::string_view m_text;
            size_t m_pos{};
            size_t m_next{};
            int32_t m_state;

            void findNext();
    };
}

#endif