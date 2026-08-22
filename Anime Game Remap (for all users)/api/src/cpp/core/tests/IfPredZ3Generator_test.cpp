// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IfPredZ3Generator -- the Z3-flavored sibling of the
// pure-Python model/iftemplate/IfPredLogicGenerator.py (which builds a sympy logic query from the
// same IfPredParser-produced tree instead).
//
// Needs Z3 (unlike SympyParser_IfPredParser_test.cpp), on top of everything IfPredParser itself
// needs. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <extern>/ordered-map/include /I <extern>/utf8proc /I <cext>/z3/include ^
//      /I <core>/src ^
//      IfPredZ3Generator_test.cpp ^
//      <core>/src/tools/StringHash.cpp <core>/src/tools/StringTools.cpp ^
//      <core>/src/tools/parsing/Token.cpp <core>/src/tools/parsing/ParseContext.cpp ^
//      <core>/src/tools/parsing/SyntaxErr.cpp ^
//      <core>/src/tools/idGenerator/UuidIdGenerator.cpp ^
//      <core>/src/model/iftemplate/IfPredParser.cpp ^
//      <core>/src/model/iftemplate/IfPredZ3Generator.cpp ^
//      <core>/src/tools/z3/Z3Context.cpp <core>/src/tools/z3/Z3Predicate.cpp ^
//      <extern>/utf8proc/utf8proc.c ^
//      /link /LIBPATH:<cext>/z3/lib libz3.lib /Fe:test.exe
//
//   (and libz3.dll needs to be next to test.exe, or on PATH, to actually run it)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/iftemplate/IfPredParser.h"
#include "AGRemapCore/model/iftemplate/IfPredZ3Generator.h"
#include "AGRemapCore/tools/parsing/Token.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using AGRemapCore::IfPredParser;
using AGRemapCore::IfPredZ3Generator;
using AGRemapCore::Token;
using AGRemapCore::Z3Context;
using AGRemapCore::Z3Predicate;

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

bool contains(const std::string& haystack, const char* needle) {
    return haystack.find(needle) != std::string::npos;
}

void test_singleVariable() {
    std::printf("test_singleVariable: \"$myVar\"\n");

    IfPredParser parser;
    auto tree = parser.parse({Token("ID", "$myVar", 1, 1)});

    Z3Context ctx;
    Z3Predicate pred = IfPredZ3Generator::generate(tree, ctx);
    std::string s = pred.toString();
    std::printf("    -> %s\n", s.c_str());

    // A bare variable, used where a predicate is expected, means "$myVar != 0"
    check(contains(s, "myVar"), "result mentions the variable");
    check(contains(s, "not") || contains(s, "distinct") || contains(s, "="), "result is some comparison-shaped predicate");
}

void test_equality() {
    std::printf("test_equality: \"$a == 1\"\n");

    IfPredParser parser;
    auto tree = parser.parse({
        Token("ID", "$a", 1, 1),
        Token("EQ", "==", 1, 3),
        Token("INT", "1", 1, 6),
    });

    Z3Context ctx;
    Z3Predicate pred = IfPredZ3Generator::generate(tree, ctx);
    std::string s = pred.toString();
    std::printf("    -> %s\n", s.c_str());

    check(contains(s, "="), "result is an equality");
    check(contains(s, "a"), "result mentions the variable");
    check(contains(s, "1"), "result mentions the literal");
}

void test_andOfComparisons() {
    std::printf("test_andOfComparisons: \"$a == 1 && $b != 2\"\n");

    IfPredParser parser;
    auto tree = parser.parse({
        Token("ID", "$a", 1, 1),
        Token("EQ", "==", 1, 3),
        Token("INT", "1", 1, 6),
        Token("AND", "&&", 1, 8),
        Token("ID", "$b", 1, 11),
        Token("NE", "!=", 1, 13),
        Token("INT", "2", 1, 16),
    });

    Z3Context ctx;
    Z3Predicate pred = IfPredZ3Generator::generate(tree, ctx);
    std::string s = pred.toString();
    std::printf("    -> %s\n", s.c_str());

    check(contains(s, "and"), "result is an 'and'");
}

void test_arithmeticInsideComparison() {
    std::printf("test_arithmeticInsideComparison: \"$a + 1 == 2\"\n");

    IfPredParser parser;
    auto tree = parser.parse({
        Token("ID", "$a", 1, 1),
        Token("PLUS", "+", 1, 4),
        Token("INT", "1", 1, 6),
        Token("EQ", "==", 1, 8),
        Token("INT", "2", 1, 11),
    });

    Z3Context ctx;
    // simplify=false so "+ a 1" stays visible rather than Z3 folding into "(= a 1)"
    Z3Predicate pred = IfPredZ3Generator::generate(tree, ctx, false);
    std::string s = pred.toString();
    std::printf("    -> %s\n", s.c_str());

    check(contains(s, "+"), "result contains the addition, unsimplified");
}

void test_notOfVariable() {
    std::printf("test_notOfVariable: \"!$flag\"\n");

    IfPredParser parser;
    auto tree = parser.parse({
        Token("NOT", "!", 1, 1),
        Token("ID", "$flag", 1, 2),
    });

    Z3Context ctx;
    Z3Predicate pred = IfPredZ3Generator::generate(tree, ctx, false);
    std::string s = pred.toString();
    std::printf("    -> %s\n", s.c_str());

    check(contains(s, "not"), "result is negated");
}

void test_emptyPredicate() {
    std::printf("test_emptyPredicate: \"\" (start empty)\n");

    IfPredParser parser;
    auto tree = parser.parse({});

    Z3Context ctx;
    Z3Predicate pred = IfPredZ3Generator::generate(tree, ctx);
    std::string s = pred.toString();
    std::printf("    -> %s\n", s.c_str());

    check(s == "false", "an empty predicate generates the literal 'false'");
}

void test_sameVariableNameReusedAcrossCalls() {
    std::printf("test_sameVariableNameReusedAcrossCalls: same ctx, two separate generate() calls\n");

    IfPredParser parser;
    auto tree1 = parser.parse({Token("ID", "$shared", 1, 1)});
    auto tree2 = parser.parse({Token("ID", "$shared", 1, 1)});

    Z3Context ctx;
    Z3Predicate pred1 = IfPredZ3Generator::generate(tree1, ctx, false);
    Z3Predicate pred2 = IfPredZ3Generator::generate(tree2, ctx, false);

    // Same variable name in the same context => syntactically identical generated predicate,
    // with no explicit 'vars' map needed (unlike the pure-Python/sympy version) -- Z3 interns
    // same-named/same-sorted constants within one context on its own.
    check(pred1.toString() == pred2.toString(), "two independent generate() calls agree on the same variable's expr");
}

} // namespace

int main() {
    test_singleVariable();
    test_equality();
    test_andOfComparisons();
    test_arithmeticInsideComparison();
    test_notOfVariable();
    test_emptyPredicate();
    test_sameVariableNameReusedAcrossCalls();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
        return 0;
    }

    std::printf("\n%d test(s) failed.\n", failures);
    return 1;
}
