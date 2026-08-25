#ifndef AGRemapCore_TextTools_H
#define AGRemapCore_TextTools_H

#include <string>
#include <string_view>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Tools for handling text :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This is a **partial** port of the pure-Python ``TextTools`` class (``tools/TextTools.py``)
        -- only the methods needed so far (by :cpp:class:`IniNamingTools`) are included. Add more
        methods as later-ported subsystems need them
     @endrst
     */
    class TextTools {
        public:

            /**
             * @brief
             @rst
             Capitalizes the first `Unicode`_ codepoint of 'txt', leaving the rest untouched --
             matches Python's ``txt[0].upper() + txt[1:]`` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Uses `utf8proc`_'s simple per-codepoint case mapping (:cpp:func:`utf8proc_toupper`),
                the same as the rest of this codebase's Unicode handling -- this does not perform
                full Unicode special-casing (eg. German ``"ß"`` uppercasing to ``"SS"``, 2
                codepoints from 1), matching Python's own ``str.upper()`` only for the common case
             @endrst
             *
             * @param txt The text to capitalize
             *
             * @return The capitalized text
             *
             * @throws std::runtime_error if 'txt' is not valid UTF-8
             */
            static std::string capitalize(std::string_view txt);

            /**
             * @brief
             @rst
             Reverses 'txt' by `Unicode`_ codepoint (not by byte, and not by `grapheme`_) -- matches
             Python's ``txt[::-1]``, including that it can break apart a multi-codepoint `grapheme`_
             (eg. a base letter followed by a combining accent) the same way Python's version does
             @endrst
             *
             * @param txt The text to reverse
             *
             * @return The reversed text
             *
             * @throws std::runtime_error if 'txt' is not valid UTF-8
             */
            static std::string reverse(std::string_view txt);
    };
}

#endif
