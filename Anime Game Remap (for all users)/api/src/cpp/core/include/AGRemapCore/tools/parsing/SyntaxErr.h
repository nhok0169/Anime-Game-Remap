#ifndef AGRemapCore_SyntaxErr_H
#define AGRemapCore_SyntaxErr_H

#include <stdexcept>
#include <string>

#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/parsing/Token.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Thrown when the syntax for some language is incorrect :raw-html:`<br />` :raw-html:`<br />`

     A minimal, Python-free data carrier for the ``ctx``/``token``/``process`` that caused the
     error -- it deliberately does *not* reimplement error-message formatting (the line/char/file
     location report a caller would want to show a user). The pybind11 binding
     that catches this (see ``PyBaseTokenizer.cpp``) constructs the real
     ``FixRaidenBoss2.exceptions.SyntaxErr.SyntaxErr`` Python object from :cpp:func:`ctx` /
     :cpp:func:`token` / :cpp:func:`process` instead, so that logic keeps living in exactly one
     place (the pure-Python class) rather than being duplicated here
     @endrst
     */
    class SyntaxErr : public std::runtime_error {
        public:

            /**
             * @brief Constructs a new syntax error
             *
             * @param ctx The context for parsing some text
             * @param token The token that caused the error
             * @param process The name of the process that caused the error
             */
            SyntaxErr(ParseContext ctx, Token token, std::string process = "parsing");

            /**
             * @brief The context for parsing some text
             */
            const ParseContext& ctx() const;

            /**
             * @brief The token that caused the error
             */
            const Token& token() const;

            /**
             * @brief The name of the process that caused the error
             */
            const std::string& process() const;

        private:
            ParseContext ctx_;
            Token token_;
            std::string process_;

            static std::string buildMessage(const Token& token, const std::string& process);
    };
}

#endif
