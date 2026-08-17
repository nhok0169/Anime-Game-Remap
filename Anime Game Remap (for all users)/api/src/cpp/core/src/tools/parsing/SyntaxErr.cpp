#include "AGRemapCore/tools/parsing/SyntaxErr.h"


namespace AGRemapCore {
    std::string SyntaxErr::buildMessage(const Token& token, const std::string& process) {
        return "Invalid string, \"" + token.val + "\", found during " + process;
    }

    SyntaxErr::SyntaxErr(ParseContext ctx, Token token, std::string process):
        std::runtime_error(buildMessage(token, process)), ctx_(std::move(ctx)), token_(std::move(token)), process_(std::move(process)) {

    }

    const ParseContext& SyntaxErr::ctx() const {
        return ctx_;
    }

    const Token& SyntaxErr::token() const {
        return token_;
    }

    const std::string& SyntaxErr::process() const {
        return process_;
    }
}
