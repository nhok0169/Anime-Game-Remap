#ifndef AGRemapCore_Token_H
#define AGRemapCore_Token_H

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

#include <optional>
#include <string>


namespace AGRemapCore {

    /**
     * @brief A token when parsing some language
     */
    class Token {
        public:

            /**
             * @brief Constructs a new token
             *
             * @param type The name for the type of token, if available
             * @param val The value of the token
             * @param lineNo The line number the token belongs to
             * @param charNo The character number the token belongs to within some line
             */
            Token(std::optional<std::string> type, std::string val, size_t lineNo, size_t charNo);

            /**
             * @brief The name for the type of token, if available
             */
            std::optional<std::string> type;

            /**
             * @brief The value of the token
             */
            std::string val;

            /**
             * @brief The line number the token belongs to
             */
            size_t lineNo;

            /**
             * @brief The character number the token belongs to within some line
             */
            size_t charNo;
    };

}

#endif
