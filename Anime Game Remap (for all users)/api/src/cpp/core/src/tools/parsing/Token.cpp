#include "AGRemapCore/tools/parsing/Token.h"


namespace AGRemapCore {
    Token::Token(std::optional<std::string> type, std::string val, size_t lineNo, size_t charNo):
        type(std::move(type)), val(std::move(val)), lineNo(lineNo), charNo(charNo) {

    }
}
