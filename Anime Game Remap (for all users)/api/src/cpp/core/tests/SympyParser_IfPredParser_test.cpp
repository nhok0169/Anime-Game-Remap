// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::SympyParser / AGRemapCore::IfPredParser --
// the C++ port of model/iftemplate/SympyParser.py / model/iftemplate/IfPredParser.py.
// Both are thin, fixed-grammar subclasses of BaseSLR1Parser<std::string> (already
// covered on its own in BaseSLR1Parser_test.cpp) -- this file focuses on what's new
// here specifically: that the real, full-size grammars build a working SLR(1) DFA
// via the default UuidIdGenerator (std::string Id, no explicit generator supplied,
// matching real usage), and that a token sequence actually parses/fails correctly
// through the resulting grammar.
//
// This file has NO dependency on the project's build system (CMake/pybind11) or Z3,
// but DOES need utf8proc (transitively, via ParseContext -> StringTools::splitlines)
// and the vendored tsl::ordered_map headers. Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <extern>/ordered-map/include /I <extern>/utf8proc ^
//      SympyParser_IfPredParser_test.cpp ^
//      <core>/src/tools/StringHash.cpp <core>/src/tools/StringTools.cpp ^
//      <core>/src/tools/parsing/Token.cpp <core>/src/tools/parsing/ParseContext.cpp ^
//      <core>/src/tools/parsing/SyntaxErr.cpp ^
//      <core>/src/tools/idGenerator/UuidIdGenerator.cpp ^
//      <core>/src/model/iftemplate/SympyParser.cpp <core>/src/model/iftemplate/IfPredParser.cpp ^
//      <extern>/utf8proc/utf8proc.c ^
//      /Fe:test.exe
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/iftemplate/IfPredParser.h"
#include "AGRemapCore/model/iftemplate/SympyParser.h"
#include "AGRemapCore/tools/parsing/SyntaxErr.h"
#include "AGRemapCore/tools/parsing/Token.h"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

using AGRemapCore::IfPredParser;
using AGRemapCore::SympyParser;
using AGRemapCore::SyntaxErr;
using AGRemapCore::Token;

namespace {

int failures = 0;

bool check(bool condition, const char* description) {
    if (condition) {
        std::printf("  [PASS] %s\n", description);
    } else {
        std::printf("  [FAIL] %s\n", description);
        ++failures;
    }
    return condition;
}

// "$var" -> ID, reduced all the way up: term -> nterm -> multexpr -> addexpr -> keyexpr -> test -> pred
std::vector<Token> singleIdToken() {
    return {Token("ID", "myVar", 1, 1)};
}

// "$a$ + $b$ * $c$" shaped: ID PLUS ID STAR ID, exercising addexpr/multexpr precedence chaining
std::vector<Token> arithmeticTokens() {
    return {
        Token("ID", "a", 1, 1),
        Token("PLUS", "+", 1, 2),
        Token("ID", "b", 1, 3),
        Token("STAR", "*", 1, 4),
        Token("ID", "c", 1, 5),
    };
}

template <typename Parser>
void checkBuiltGrammar(Parser& parser, const char* label) {
    char desc[128];

    std::snprintf(desc, sizeof(desc), "%s: constructs and sets up without throwing", label);
    check(true, desc); // reaching here means the constructor (setup=true) didn't throw

    std::snprintf(desc, sizeof(desc), "%s: nonTermSymbols includes every grammar nonterminal", label);
    auto nonTerms = parser.nonTermSymbols();
    bool hasAll = nonTerms.count("pred_prime") && nonTerms.count("pred") && nonTerms.count("test")
                  && nonTerms.count("keyexpr") && nonTerms.count("addexpr") && nonTerms.count("multexpr")
                  && nonTerms.count("nterm") && nonTerms.count("term");
    check(hasAll, desc);

    std::snprintf(desc, sizeof(desc), "%s: constructDFA (via setup) produced a non-trivial DFA", label);
    check(parser.dfa().stateSize() > 1, desc);

    std::snprintf(desc, sizeof(desc), "%s: constructDFA (via setup) produced reduce actions", label);
    check(!parser.reductions().empty(), desc);
}

template <typename Parser>
void checkParsesSingleId(Parser& parser, const char* label) {
    char desc[128];
    bool threw = false;
    try {
        auto tree = parser.parse(singleIdToken());
        std::snprintf(desc, sizeof(desc), "%s: a single ID token parses to a tree rooted at a real reduction", label);
        check(tree.getNode(tree.rootId(), false) != nullptr, desc);
    } catch (const SyntaxErr&) {
        threw = true;
    }
    std::snprintf(desc, sizeof(desc), "%s: a single ID token (term->nterm->...->pred chain) parses without a SyntaxErr", label);
    check(!threw, desc);
}

template <typename Parser>
void checkParsesArithmetic(Parser& parser, const char* label) {
    char desc[128];
    bool threw = false;
    try {
        parser.parse(arithmeticTokens());
    } catch (const SyntaxErr&) {
        threw = true;
    }
    std::snprintf(desc, sizeof(desc), "%s: 'a + b * c' (PLUS/STAR precedence chaining) parses without a SyntaxErr", label);
    check(!threw, desc);
}

template <typename Parser>
void checkRejectsGarbage(Parser& parser, const char* label) {
    char desc[128];
    bool threw = false;
    try {
        // PLUS can never start a 'pred' -- no production has it as (or derives to) a leading symbol
        parser.parse({Token("PLUS", "+", 1, 1)});
    } catch (const SyntaxErr&) {
        threw = true;
    }
    std::snprintf(desc, sizeof(desc), "%s: a token with no valid production (leading PLUS) raises SyntaxErr", label);
    check(threw, desc);
}

void test_ifPredParser() {
    std::printf("test_ifPredParser\n");

    IfPredParser parser;

    checkBuiltGrammar(parser, "IfPredParser");
    checkParsesSingleId(parser, "IfPredParser");
    checkParsesArithmetic(parser, "IfPredParser");
    checkRejectsGarbage(parser, "IfPredParser");
}

void test_sympyParser() {
    std::printf("test_sympyParser\n");

    SympyParser parser;
    checkBuiltGrammar(parser, "SympyParser");
    checkParsesSingleId(parser, "SympyParser");
    checkParsesArithmetic(parser, "SympyParser");
    checkRejectsGarbage(parser, "SympyParser");

    // SympyParser-only productions (AND/OR function-call form, e.g. "AndFunc(...)") that
    // IfPredParser's grammar doesn't have -- exercise one to confirm it's really reachable.
    std::vector<Token> andFuncCall = {
        Token("ANDFUNC", "And", 1, 1),
        Token("LPAREN", "(", 1, 2),
        Token("ID", "x", 1, 3),
        Token("COMMA", ",", 1, 4),
        Token("ID", "y", 1, 5),
        Token("RPAREN", ")", 1, 6),
    };
    bool threw = false;
    try {
        parser.parse(andFuncCall);
    } catch (const SyntaxErr&) {
        threw = true;
    }
    check(!threw, "SympyParser: 'And(x, y)' (the sympy-only ANDFUNC production) parses without a SyntaxErr");
}

} // namespace

int main() {
    test_ifPredParser();
    test_sympyParser();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
        return 0;
    }

    std::printf("\n%d test(s) failed.\n", failures);
    return 1;
}
