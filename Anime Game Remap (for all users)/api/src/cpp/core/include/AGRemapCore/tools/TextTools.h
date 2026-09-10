#ifndef AGRemapCore_TextTools_H
#define AGRemapCore_TextTools_H

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
