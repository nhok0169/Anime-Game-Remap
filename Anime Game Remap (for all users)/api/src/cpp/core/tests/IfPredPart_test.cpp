// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IfPredPart / IfPredPartTypeTools -- the standalone,
// Z3-flavored C++ counterpart to the pure-Python model/iftemplate/IfPredPart.py (see IfPredPart.h's
// own comment for why this is a new, separate class rather than a replacement of the Python one).
//
// Compile directly, e.g. (after `vcvarsall.bat x64`) -- same shape as
// Z3IfPredGenerator_test.cpp's own header comment, plus IfPredPartType.cpp/IfPredPart.cpp:
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <core>/src /I <extern>/ordered-map/include /I <extern>/utf8proc /I <cext>/z3/include ^
//      IfPredPart_test.cpp ^
//      <core>/src/tools/StringHash.cpp <core>/src/tools/StringTools.cpp ^
//      <core>/src/tools/grapheme/GraphemeIterator.cpp <core>/src/tools/grapheme/GraphemeRange.cpp ^
//      <core>/src/tools/parsing/Token.cpp <core>/src/tools/parsing/ParseContext.cpp ^
//      <core>/src/tools/parsing/SyntaxErr.cpp <core>/src/tools/parsing/BaseTokenizer.cpp ^
//      <core>/src/tools/parsing/FilteredTokenizer.cpp <core>/src/tools/parsing/IfPredTokenizer.cpp ^
//      <core>/src/tools/idGenerator/UuidIdGenerator.cpp ^
//      <core>/src/model/iftemplate/IfTemplatePart.cpp ^
//      <core>/src/model/iftemplate/IfPredParser.cpp ^
//      <core>/src/model/iftemplate/IfPredZ3Generator.cpp <core>/src/model/iftemplate/Z3IfPredGenerator.cpp ^
//      <core>/src/model/iftemplate/IfPredPart.cpp <core>/src/constants/IfPredPartType.cpp ^
//      <core>/src/tools/z3/Z3Context.cpp <core>/src/tools/z3/Z3Predicate.cpp ^
//      <extern>/utf8proc/utf8proc.c ^
//      /Fe:ifpredpart_test.exe ^
//      /link /LIBPATH:<cext>/z3/lib libz3.lib
// -----------------------------------------------------------------------------

#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"

// Manual verification only -- reaches past the opaque wrappers via the same test-only hooks
// Z3IfPredGenerator_test.cpp uses, to build a solver directly for semantic-equivalence checks.
#include "tools/z3/Z3Internal.h"

#include <cstdio>
#include <optional>
#include <string>

using AGRemapCore::IfPredPart;
using AGRemapCore::IfPredPartType;
using AGRemapCore::IfPredPartTypeTools;
using AGRemapCore::ParseContext;
using AGRemapCore::Z3Context;
using AGRemapCore::Z3Predicate;

namespace {

int failures = 0;

bool check(bool condition, const std::string& description) {
    if (condition) {
        std::printf("  [PASS] %s\n", description.c_str());
    } else {
        std::printf("  [FAIL] %s\n", description.c_str());
        ++failures;
    }
    return condition;
}

bool provablyEquivalent(Z3Context& ctxWrapper, const Z3Predicate& p1, const Z3Predicate& p2) {
    z3::context& ctx = *AGRemapCore::getZ3ContextImplForTesting(ctxWrapper).ctx;
    z3::solver solver(ctx);
    solver.add(AGRemapCore::getZ3PredicateImplForTesting(p1).predicate != AGRemapCore::getZ3PredicateImplForTesting(p2).predicate);
    return solver.check() == z3::unsat;
}

Z3Predicate handBuilt(Z3Context& ctxWrapper, z3::expr e) {
    z3::expr b = e.is_bool() ? e : (e != e.ctx().real_val(0));
    return AGRemapCore::makeZ3PredicateForTesting(std::make_unique<Z3Predicate::Impl>(b, AGRemapCore::getZ3ContextImplForTesting(ctxWrapper).ctx));
}

void test_getType() {
    std::printf("getType\n");

    check(IfPredPartTypeTools::getType("if $x == 5") == IfPredPartType::If, "'if ...' -> If");
    check(IfPredPartTypeTools::getType("  ENDIF  ") == IfPredPartType::EndIf, "'  ENDIF  ' -> EndIf (case/whitespace insensitive)");
    check(IfPredPartTypeTools::getType("elif $x") == IfPredPartType::Elif, "'elif ...' -> Elif");
    check(IfPredPartTypeTools::getType("else if $x") == IfPredPartType::Elif, "'else if ...' -> Elif");
    check(IfPredPartTypeTools::getType("else") == IfPredPartType::Else, "'else' -> Else");
    check(!IfPredPartTypeTools::getType("somethingElse").has_value(), "unrelated text -> nullopt");
}

void test_getTestStr() {
    std::printf("getTestStr\n");

    Z3Context ctx;

    IfPredPart ifPart("if $x == 5 then", IfPredPartType::If, ctx);
    check(ifPart.getTestStr() == " $x == 5 ", "If: leading 'if' and trailing 'then' stripped, surrounding text preserved");

    // The pure-Python original has a live bug here: for a bare "elif ..." spelling (as opposed to
    // "else if ..."), its getTestStr() falls into a branch referencing a Python local ('result')
    // that was never assigned on that path, raising UnboundLocalError. This C++ port implements
    // the clearly-intended behavior (strip the leading 'elif' keyword) instead of reproducing a
    // crash -- see this test file's own PR/commit message for the full note.
    IfPredPart bareElifPart("elif $x == 5", IfPredPartType::Elif, ctx);
    check(bareElifPart.getTestStr() == " $x == 5", "Elif ('elif ...' spelling): leading 'elif' stripped without crashing");

    IfPredPart elseIfPart("else if $x == 5", IfPredPartType::Elif, ctx);
    check(elseIfPart.getTestStr() == "  $x == 5", "Elif ('else if ...' spelling): leading 'else'/'if' both stripped");
}

void test_constructionAndQuery() {
    std::printf("constructionAndQuery\n");

    Z3Context ctx;
    z3::context& rawCtx = *AGRemapCore::getZ3ContextImplForTesting(ctx).ctx;

    IfPredPart ifPart("if $x == 5 then", IfPredPartType::If, ctx);
    check(ifPart.query.has_value(), "If: query is populated");
    if (ifPart.query.has_value()) {
        Z3Predicate expected = handBuilt(ctx, rawCtx.real_const("$x") == rawCtx.real_val(5));
        check(provablyEquivalent(ctx, *ifPart.query, expected), "If: query is provably '$x == 5'");
    }

    IfPredPart elifPart("elif $y != 3", IfPredPartType::Elif, ctx);
    check(elifPart.query.has_value(), "Elif (bare spelling): query is populated");
    if (elifPart.query.has_value()) {
        Z3Predicate expected = handBuilt(ctx, rawCtx.real_const("$y") != rawCtx.real_val(3));
        check(provablyEquivalent(ctx, *elifPart.query, expected), "Elif (bare spelling): query is provably '$y != 3'");
    }

    IfPredPart elsePart("else", IfPredPartType::Else, ctx);
    check(elsePart.query.has_value(), "Else: query is populated");
    if (elsePart.query.has_value()) {
        check(provablyEquivalent(ctx, *elsePart.query, handBuilt(ctx, rawCtx.bool_val(true))), "Else: query is provably 'true'");
    }

    IfPredPart endifPart("endif", IfPredPartType::EndIf, ctx);
    check(!endifPart.query.has_value(), "EndIf: query is empty");
}

void test_malformedPredicateReturnsNullopt() {
    std::printf("malformedPredicateReturnsNullopt\n");

    Z3Context ctx;
    // A leading PLUS can never start a 'pred' -- guaranteed SyntaxErr during parse (same fixture
    // IfPredZ3Generator_test.cpp/SympyParser_IfPredParser_test.cpp already rely on).
    IfPredPart garbage("if + then", IfPredPartType::If, ctx);
    check(!garbage.query.has_value(), "unparseable predicate text: query is empty, no exception propagates");
}

void test_ctxMutation() {
    std::printf("ctxMutation\n");

    Z3Context z3ctx;
    ParseContext parseCtx("placeholder", "myFile.ini", 7);
    IfPredPart part("if $x == 5 then", IfPredPartType::If, z3ctx, &parseCtx);

    check(parseCtx.lines.size() == 1 && parseCtx.lines.at(0) == " $x == 5 ", "supplied ParseContext's lines are replaced with getTestStr()'s result");
    check(parseCtx.file.has_value() && *parseCtx.file == "myFile.ini", "supplied ParseContext's file/startLineNo are left untouched");
}

void test_clone() {
    std::printf("clone\n");

    Z3Context ctx;
    IfPredPart original("if $x == 5 then", IfPredPartType::If, ctx);

    IfPredPart sameId = original.clone(false);
    check(sameId.id() == original.id(), "clone(false): id is preserved");
    check(sameId.src == original.src && sameId.type == original.type, "clone(false): src/type are preserved");
    check(sameId.query.has_value() == original.query.has_value(), "clone(false): query presence is preserved");
    if (sameId.query.has_value() && original.query.has_value()) {
        check(provablyEquivalent(ctx, *sameId.query, *original.query), "clone(false): query is provably equivalent to the original's");
    }

    IfPredPart newId = original.clone(true);
    check(newId.id() != original.id(), "clone(true): a fresh id is generated");
}

void test_getIfPredStr() {
    std::printf("getIfPredStr\n");

    Z3Context ctx;
    z3::context& rawCtx = *AGRemapCore::getZ3ContextImplForTesting(ctx).ctx;

    Z3Predicate pred = handBuilt(ctx, rawCtx.real_const("$x") == rawCtx.real_val(5));
    std::optional<std::string> text = IfPredPart::getIfPredStr(pred);
    check(text.has_value(), "getIfPredStr: succeeds for a representable predicate");
    if (text.has_value()) {
        std::printf("    -> %s\n", text->c_str());
        check(text->find("$x") != std::string::npos, "getIfPredStr: result mentions the variable");
    }

    // A general (non 0/1) ite has no .ini equivalent -- IfPredZ3Generator::generate throws;
    // getIfPredStr should swallow it and return std::nullopt, matching the Python original's own
    // try/except-returns-None shape.
    Z3Predicate unrepresentable = handBuilt(ctx, z3::ite(rawCtx.bool_const("$flag"), rawCtx.real_val(5), rawCtx.real_val(9)) != rawCtx.real_val(0));
    check(!IfPredPart::getIfPredStr(unrepresentable).has_value(), "getIfPredStr: returns std::nullopt for an unrepresentable predicate");
}

void test_toStr() {
    std::printf("toStr\n");

    Z3Context ctx;
    IfPredPart part("   if $x == 5 then\n", IfPredPartType::If, ctx);

    check(part.toStr() == "   if $x == 5 then", "toStr(): src as-is, minus a single trailing newline");
    check(part.toStr("::") == "::if $x == 5 then", "toStr(linePrefix): left spacing stripped, prefix prepended");
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    test_getType();
    test_getTestStr();
    test_constructionAndQuery();
    test_malformedPredicateReturnsNullopt();
    test_ctxMutation();
    test_clone();
    test_getIfPredStr();
    test_toStr();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
        return 0;
    }

    std::printf("\n%d test(s) failed.\n", failures);
    return 1;
}
