#ifndef AGRemapCore_Token_H
#define AGRemapCore_Token_H

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
