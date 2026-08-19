#ifndef AGRemapCore_StringTools_H
#define AGRemapCore_StringTools_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief Tools for handling with strings
     */
    class StringTools {
        public:

            /**
             * @brief
             @rst
             `compare function`_ for 2 string pointers
             @endrst
             * 
             * @param strPtr1 The first string pointer in the comparison
             * @param strPtr2 The second string pointer in the comparison
             * 
             * @return The comparison result
             */
            static std::int8_t compareStrPtrs(const std::string *strPtr1, const std::string *strPtr2);

            /**
             * @brief Counts the number of `graphemes`_ in a string
             * 
             * @param txt The text to count
             * 
             * @return The resultant count
             */
            static size_t countGrapheme(const std::string &txt);

            /**
             * @brief Splits a string into different lines
             *
             * @param txt The text to split
             *
             * @return The resultant split lines
             */
            [[nodiscard]] static std::vector<std::string_view> splitlines(std::string_view txt);

            /**
             * @brief
             @rst
             Strips whitespace from both ends of a string, similar to `Python's str.strip`_
             @endrst
             *
             * @param txt The text to strip
             *
             * @return The resultant stripped text
             */
            [[nodiscard]] static std::string_view strip(std::string_view txt);
    };
}


#endif