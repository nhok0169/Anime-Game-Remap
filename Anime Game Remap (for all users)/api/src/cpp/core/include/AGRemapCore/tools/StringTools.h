#ifndef AGRemapCore_StringTools_H
#define AGRemapCore_StringTools_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Tools for handling with strings :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Every text operation in this class works on `graphemes`_ (what a person sees as one
        character) rather than on bytes or on individual `Unicode`_ codepoints, so emojis,
        combining marks, ZWJ sequences and other multi-codepoint characters are never split apart
        or miscounted. Unicode-aware behaviour (whitespace, case) is provided by `utf8proc`_.

     .. warning::
        Text is expected to be UTF-8. A byte that is not valid UTF-8 is treated as a `grapheme`_ of
        its own that is neither whitespace nor cased, and is passed through unchanged -- these
        methods never throw on malformed input
     @endrst
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
            static size_t countGrapheme(std::string_view txt);

            /**
             * @brief
             @rst
             Whether 'grapheme' is made up entirely of whitespace, similar to `Python's str.isspace`_ :raw-html:`<br />` :raw-html:`<br />`

             A codepoint counts as whitespace under the same rule `Python`_ uses: its bidirectional
             class is ``WS``, ``B`` or ``S``, or its category is ``Zs``. This covers the `ASCII`_
             whitespace characters as well as eg. the no-break space (``U+00A0``), the ideographic
             space (``U+3000``) and the line/paragraph separators (``U+2028``/``U+2029``)
             @endrst
             *
             * @param grapheme The text to check. Usually a single `grapheme`_, but any text is accepted
             *
             * @return Whether every codepoint in 'grapheme' is whitespace. The empty string is not whitespace
             */
            static bool isSpace(std::string_view grapheme);

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
             Strips whitespace from both ends of a string, similar to `Python's str.strip`_ :raw-html:`<br />` :raw-html:`<br />`

             Whitespace is decided per `grapheme`_ by :cpp:func:`isSpace`, so `Unicode`_ whitespace is
             stripped too, and a whitespace codepoint that is part of a larger `grapheme`_ (eg. a
             space carrying a combining mark) is left alone
             @endrst
             *
             * @param txt The text to strip
             *
             * @return The resultant stripped text
             */
            [[nodiscard]] static std::string_view strip(std::string_view txt);

            /**
             * @brief
             @rst
             Strips whitespace from the start of a string, similar to `Python's str.lstrip`_.
             See :cpp:func:`strip` for what counts as whitespace
             @endrst
             *
             * @param txt The text to strip
             *
             * @return The resultant stripped text
             */
            [[nodiscard]] static std::string_view lstrip(std::string_view txt);

            /**
             * @brief
             @rst
             Strips whitespace from the end of a string, similar to `Python's str.rstrip`_.
             See :cpp:func:`strip` for what counts as whitespace
             @endrst
             *
             * @param txt The text to strip
             *
             * @return The resultant stripped text
             */
            [[nodiscard]] static std::string_view rstrip(std::string_view txt);

            /**
             * @brief
             @rst
             Lowercases every codepoint of 'txt', similar to `Python's str.lower`_ :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Uses `utf8proc`_'s simple per-codepoint case mapping (:cpp:func:`utf8proc_tolower`),
                the same as :cpp:func:`TextTools::capitalize` -- this does not perform full Unicode
                special-casing (eg. ``"İ"`` lowercasing to ``"i̇"``, 2 codepoints from 1), matching
                `Python`_'s own ``str.lower()`` for all but those few characters. `Grapheme`_ boundaries
                are unaffected by the mapping, so the result has the same number of `graphemes`_
                as 'txt'
             @endrst
             *
             * @param txt The text to lowercase
             *
             * @return The lowercased text
             */
            [[nodiscard]] static std::string toLower(std::string_view txt);

            /**
             * @brief
             @rst
             Retrieves the first few `graphemes`_ of 'txt', similar to `Python`_'s ``txt[:count]``
             @endrst
             *
             * @param txt The text to take from
             * @param count How many `graphemes`_ to take
             *
             * @return The prefix of 'txt' holding its first 'count' `graphemes`_, or all of 'txt' if it has fewer than 'count' `graphemes`_
             */
            [[nodiscard]] static std::string_view firstGraphemes(std::string_view txt, size_t count);

            /**
             * @brief
             @rst
             Retrieves the last few `graphemes`_ of 'txt', similar to `Python`_'s ``txt[-count:]``
             @endrst
             *
             * @param txt The text to take from
             * @param count How many `graphemes`_ to take
             *
             * @return The suffix of 'txt' holding its last 'count' `graphemes`_, or all of 'txt' if it has fewer than 'count' `graphemes`_
             */
            [[nodiscard]] static std::string_view lastGraphemes(std::string_view txt, size_t count);

            /**
             * @brief
             @rst
             Whether 'txt' starts with 'prefix', compared by whole `graphemes`_ :raw-html:`<br />` :raw-html:`<br />`

             Unlike a byte-wise ``std::string_view::starts_with``, a prefix never matches only part of
             a `grapheme`_: ``"e"`` is not a prefix of ``"é"`` written as ``"e"`` + a combining acute
             @endrst
             *
             * @param txt The text to check
             * @param prefix The prefix to look for
             *
             * @return Whether 'txt' starts with 'prefix'
             */
            static bool startsWith(std::string_view txt, std::string_view prefix);

            /**
             * @brief
             @rst
             Whether 'txt' ends with 'suffix', compared by whole `graphemes`_.
             See :cpp:func:`startsWith` for how this differs from a byte-wise check
             @endrst
             *
             * @param txt The text to check
             * @param suffix The suffix to look for
             *
             * @return Whether 'txt' ends with 'suffix'
             */
            static bool endsWith(std::string_view txt, std::string_view suffix);

            /**
             * @brief Whether 2 strings are equal, ignoring case (see #toLower for how case is mapped)
             *
             * @param txt1 The first text to compare
             * @param txt2 The second text to compare
             *
             * @return Whether both texts are equal ignoring case
             */
            static bool equalsIgnoreCase(std::string_view txt1, std::string_view txt2);

            /**
             * @brief Whether 'txt' ends with 'suffix', compared by whole `graphemes`_ and ignoring case (see #toLower for how case is mapped)
             *
             * @param txt The text to check
             * @param suffix The suffix to look for
             *
             * @return Whether 'txt' ends with 'suffix' ignoring case
             */
            static bool endsWithIgnoreCase(std::string_view txt, std::string_view suffix);

            /**
             * @brief Erases every occurrence of 'target' from 'txt', in place
             *
             * @param txt The string to erase from
             * @param target The substring to erase. Erasing the empty string is a no-op
             */
            static void eraseAll(std::string& txt, std::string_view target);
    };
}


#endif
