#include "AGRemapCore/model/iftemplate/SympyParser.h"

#include <utility>


namespace AGRemapCore {

    namespace {
        // Ported verbatim (same keys, same order, same productions) from the pure-Python
        // SympyParser's own 'productions' dict literal.
        BaseSLR1Parser<std::string>::Productions makeProductions(const std::string& startSymbol, const std::string& startToken, const std::string& endToken) {
            return {
                {"start",             {startSymbol, {startToken, "pred", endToken}}},
                {"start empty",       {startSymbol, {startToken, endToken}}},
                {"pred reduce",       {"pred", {"test"}}},
                {"and",               {"pred", {"pred", "AND", "test"}}},
                {"or",                {"pred", {"pred", "OR", "test"}}},
                {"and func",          {"pred", {"ANDFUNC", "LPAREN", "predarglst", "RPAREN"}}},
                {"or func",           {"pred", {"ORFUNC", "LPAREN", "predarglst", "RPAREN"}}},
                {"pred arg list",     {"predarglst", {"predarglst", "COMMA", "pred"}}},
                {"pred args reduce",  {"predarglst", {"pred"}}},
                {"test reduce",       {"test", {"keyexpr"}}},
                {"eq",                {"test", {"keyexpr", "EQ", "keyexpr"}}},
                {"ne",                {"test", {"keyexpr", "NE", "keyexpr"}}},
                {"gt",                {"test", {"keyexpr", "GT", "keyexpr"}}},
                {"ge",                {"test", {"keyexpr", "GE", "keyexpr"}}},
                {"lt",                {"test", {"keyexpr", "LT", "keyexpr"}}},
                {"le",                {"test", {"keyexpr", "LE", "keyexpr"}}},
                {"eq func",           {"test", {"EQFUNC", "LPAREN", "pred", "COMMA", "pred", "RPAREN"}}},
                {"ne func",           {"test", {"NEFUNC", "LPAREN", "pred", "COMMA", "pred", "RPAREN"}}},
                {"gt func",           {"test", {"GTFUNC", "LPAREN", "pred", "COMMA", "pred", "RPAREN"}}},
                {"ge func",           {"test", {"GEFUNC", "LPAREN", "pred", "COMMA", "pred", "RPAREN"}}},
                {"lt func",           {"test", {"LTFUNC", "LPAREN", "pred", "COMMA", "pred", "RPAREN"}}},
                {"le func",           {"test", {"LEFUNC", "LPAREN", "pred", "COMMA", "pred", "RPAREN"}}},
                {"keyexpr reduce",    {"keyexpr", {"addexpr"}}},
                {"true",              {"keyexpr", {"TRUE"}}},
                {"false",             {"keyexpr", {"FALSE"}}},
                {"addexpr reduce",    {"addexpr", {"multexpr"}}},
                {"add",               {"addexpr", {"addexpr", "PLUS", "multexpr"}}},
                {"subtract",          {"addexpr", {"addexpr", "MINUS", "multexpr"}}},
                {"multexpr reduce",   {"multexpr", {"nterm"}}},
                {"multiply",          {"multexpr", {"multexpr", "STAR", "nterm"}}},
                {"divide",            {"multexpr", {"multexpr", "SLASH", "nterm"}}},
                {"nterm reduce",      {"nterm", {"term"}}},
                {"not",               {"nterm", {"NOT", "nterm"}}},
                {"not func",          {"nterm", {"NOTFUNC", "LPAREN", "pred", "RPAREN"}}},
                {"variable",          {"term", {"ID"}}},
                {"int",               {"term", {"INT"}}},
                {"float",             {"term", {"FLOAT"}}},
                {"bracket loop",      {"term", {"LPAREN", "pred", "RPAREN"}}},
            };
        }
    }

    SympyParser::SympyParser(std::string startToken, std::string endToken, std::string nullToken, bool setup):
        // 'startToken'/'endToken' are each read here AND passed again below (as the base class's
        // own members) within the same initializer -- since parameter initialization order across
        // a single call's arguments is unspecified (C++17 [expr.call]), std::move-ing them in the
        // same call that also reads them for makeProductions() would be a real sequencing hazard
        // (the move could be sequenced before makeProductions() reads the value, leaving it empty).
        // Pass plain copies here instead; only 'nullToken' (used once) is safe to move.
        BaseSLR1Parser<std::string>(makeProductions("pred_prime", startToken, endToken), "pred_prime",
                                     startToken, endToken, std::move(nullToken), setup)
    {

    }
}
