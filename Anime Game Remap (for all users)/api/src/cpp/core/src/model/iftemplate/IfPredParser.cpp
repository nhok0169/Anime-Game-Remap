#include "AGRemapCore/model/iftemplate/IfPredParser.h"

#include <utility>


namespace AGRemapCore {

    namespace {
        // Ported verbatim (same keys, same order, same productions) from the pure-Python
        // IfPredParser's own 'productions' dict literal.
        BaseSLR1Parser<std::string>::Productions makeProductions(const std::string& startSymbol, const std::string& startToken, const std::string& endToken) {
            return {
                {"start",             {startSymbol, {startToken, "pred", endToken}}},
                {"start empty",       {startSymbol, {startToken, endToken}}},
                {"pred reduce",       {"pred", {"test"}}},
                {"and",               {"pred", {"pred", "AND", "test"}}},
                {"or",                {"pred", {"pred", "OR", "test"}}},
                {"test reduce",       {"test", {"keyexpr"}}},
                {"eq",                {"test", {"keyexpr", "EQ", "keyexpr"}}},
                {"ne",                {"test", {"keyexpr", "NE", "keyexpr"}}},
                {"gt",                {"test", {"keyexpr", "GT", "keyexpr"}}},
                {"ge",                {"test", {"keyexpr", "GE", "keyexpr"}}},
                {"lt",                {"test", {"keyexpr", "LT", "keyexpr"}}},
                {"le",                {"test", {"keyexpr", "LE", "keyexpr"}}},
                {"keyexpr reduce",    {"keyexpr", {"addexpr"}}},
                {"null",              {"keyexpr", {"NULL"}}},
                {"addexpr reduce",    {"addexpr", {"multexpr"}}},
                {"add",               {"addexpr", {"addexpr", "PLUS", "multexpr"}}},
                {"subtract",          {"addexpr", {"addexpr", "MINUS", "multexpr"}}},
                {"multexpr reduce",   {"multexpr", {"nterm"}}},
                {"multiply",          {"multexpr", {"multexpr", "STAR", "nterm"}}},
                {"divide",            {"multexpr", {"multexpr", "SLASH", "nterm"}}},
                {"nterm reduce",      {"nterm", {"term"}}},
                {"not",               {"nterm", {"NOT", "nterm"}}},
                {"variable",          {"term", {"ID"}}},
                {"int",               {"term", {"INT"}}},
                {"float",             {"term", {"FLOAT"}}},
                {"bracket loop",      {"term", {"LPAREN", "pred", "RPAREN"}}},
            };
        }
    }

    IfPredParser::IfPredParser(std::string startToken, std::string endToken, std::string nullToken, bool setup):
        // See SympyParser's identically-shaped constructor for why 'startToken'/'endToken' are
        // passed as plain copies here, not std::move-d -- they're each read again by
        // makeProductions() within this same initializer, and parameter initialization order
        // across one call's arguments is unspecified (C++17 [expr.call]).
        BaseSLR1Parser<std::string>(makeProductions("pred_prime", startToken, endToken), "pred_prime",
                                     startToken, endToken, std::move(nullToken), setup)
    {

    }
}
