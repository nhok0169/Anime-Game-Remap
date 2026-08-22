// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::Z3IfPredGenerator -- the inverse of
// IfPredZ3Generator (Z3 predicate -> .ini predicate text, rather than the other way around).
//
// The primary check here is round-trip SEMANTIC equivalence via a real z3::solver, not string
// matching -- 'text -> IfPredParser -> IfPredZ3Generator -> Z3Predicate -> Z3IfPredGenerator ->
// text' is allowed to change the text (numeral reformatting, flattened and/or chains, ...) as
// long as re-parsing it produces a *logically equivalent* predicate. Also exercises a handful of
// hand-built Z3 expressions with no IfPredZ3Generator-shaped origin at all (n-ary distinct, xor,
// implies, a bare unary minus of a variable, a non-terminating rational) to cover constructs the
// forward direction never itself produces.
//
// Compile directly, e.g. (after `vcvarsall.bat x64`) -- same shape as
// IfPredZ3Generator_test.cpp's own header comment, plus IfPredTokenizer.cpp/FilteredTokenizer.cpp/
// BaseTokenizer.cpp/Z3IfPredGenerator.cpp for the full text->text round trip:
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <core>/src /I <extern>/ordered-map/include /I <extern>/utf8proc /I <cext>/z3/include ^
//      Z3IfPredGenerator_test.cpp ^
//      <core>/src/tools/StringHash.cpp <core>/src/tools/StringTools.cpp ^
//      <core>/src/tools/grapheme/GraphemeIterator.cpp <core>/src/tools/grapheme/GraphemeRange.cpp ^
//      <core>/src/tools/parsing/Token.cpp <core>/src/tools/parsing/ParseContext.cpp ^
//      <core>/src/tools/parsing/SyntaxErr.cpp <core>/src/tools/parsing/BaseTokenizer.cpp ^
//      <core>/src/tools/parsing/FilteredTokenizer.cpp <core>/src/tools/parsing/IfPredTokenizer.cpp ^
//      <core>/src/tools/idGenerator/UuidIdGenerator.cpp ^
//      <core>/src/model/iftemplate/IfPredParser.cpp ^
//      <core>/src/model/iftemplate/IfPredZ3Generator.cpp <core>/src/model/iftemplate/Z3IfPredGenerator.cpp ^
//      <core>/src/tools/z3/Z3Context.cpp <core>/src/tools/z3/Z3Predicate.cpp ^
//      <extern>/utf8proc/utf8proc.c ^
//      /Fe:z3ifpred_test.exe ^
//      /link /LIBPATH:<cext>/z3/lib libz3.lib
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/iftemplate/IfPredParser.h"
#include "AGRemapCore/model/iftemplate/IfPredZ3Generator.h"
#include "AGRemapCore/model/iftemplate/Z3IfPredGenerator.h"
#include "AGRemapCore/tools/parsing/IfPredTokenizer.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"

// Only this .cpp (a manual verification harness, not part of the real build) reaches past the
// opaque wrappers directly -- it needs a real z3::solver to check semantic equivalence, which
// Z3Context/Z3Predicate deliberately don't expose. AGRemapCore's own source is never allowed to
// do this outside of Z3Context.cpp/Z3Predicate.cpp/IfPredZ3Generator.cpp/Z3IfPredGenerator.cpp
// themselves -- see those files' own comments.
#include "tools/z3/Z3Internal.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

using AGRemapCore::IfPredParser;
using AGRemapCore::IfPredTokenizer;
using AGRemapCore::IfPredZ3Generator;
using AGRemapCore::ParseContext;
using AGRemapCore::Z3Context;
using AGRemapCore::Z3IfPredGenerator;
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

Z3Predicate textToPredicate(const std::string& src, Z3Context& ctx) {
    IfPredTokenizer tokenizer;
    ParseContext parseCtx(src);
    auto tokens = tokenizer.simplifiedMaximalMunch(parseCtx);

    IfPredParser parser;
    auto tree = parser.parse(tokens, &parseCtx);

    return IfPredZ3Generator::generate(tree, ctx);
}

// The real correctness check: prove 'p1 iff p2' via z3, not string comparison -- ask the solver
// whether 'p1 != p2' is satisfiable; UNSAT means they're logically equivalent for every possible
// assignment of every free variable.
bool provablyEquivalent(Z3Context& ctxWrapper, const Z3Predicate& p1, const Z3Predicate& p2) {
    z3::context& ctx = *AGRemapCore::getZ3ContextImplForTesting(ctxWrapper).ctx;
    z3::solver solver(ctx);
    solver.add(AGRemapCore::getZ3PredicateImplForTesting(p1).predicate != AGRemapCore::getZ3PredicateImplForTesting(p2).predicate);
    return solver.check() == z3::unsat;
}

void checkRoundTrip(const std::string& src, const char* label) {
    std::printf("roundTrip: %s\n", label);

    Z3Context ctx;
    Z3Predicate original = textToPredicate(src, ctx);

    std::string regenerated;
    try {
        regenerated = Z3IfPredGenerator::generate(original);
    } catch (const std::exception& e) {
        check(false, std::string(label) + ": generate() threw unexpectedly (" + e.what() + ")");
        return;
    }
    std::printf("    original: %s\n", src.c_str());
    std::printf("    regenerated: %s\n", regenerated.c_str());

    Z3Predicate reparsed = textToPredicate(regenerated, ctx);

    check(provablyEquivalent(ctx, original, reparsed), std::string(label) + ": regenerated text is provably equivalent to the original");
}

void test_textRoundTrips() {
    checkRoundTrip("$swapvar == 5", "simple equality");
    checkRoundTrip("$x >= 5", "simple inequality");
    checkRoundTrip("($a == 1 && $b != 2) || !$c", "and/or/not mix");
    checkRoundTrip("$a + $b * $c - $d / $e", "arithmetic precedence chain");
    checkRoundTrip("($x + $y) * ($a - $b)", "explicit bracket-loop nesting");
    checkRoundTrip("!($z >= 1 && $z < -9 && $w == null)", "negated compound predicate");
    checkRoundTrip("$a - ($b + $c)", "non-associative subtract needing a preserved bracket");
    checkRoundTrip("$a / ($b * $c)", "non-associative divide needing a preserved bracket");
    checkRoundTrip("($x == 2) == ($y < 7.0)", "comparison-of-comparisons via bracket loop");
    checkRoundTrip("3.456", "plain float literal as a whole predicate");
    checkRoundTrip("-234", "plain negative int literal as a whole predicate");
    checkRoundTrip(" ", "empty predicate (matches test_IfPredLogicGenerator.py's own ' ' case)");
}

// Hand-built z3::expr trees with no IfPredZ3Generator-shaped origin -- covers constructs the
// forward direction never itself produces (n-ary distinct, xor, implies, a bare unary minus of a
// variable, a non-terminating rational), verified the same way: generate text, re-parse it back
// through the real tokenizer/parser/forward-generator, and prove equivalence to the hand-built
// original via the solver.
void checkHandBuilt(Z3Context& ctxWrapper, const z3::expr& handBuilt, const char* label) {
    std::printf("handBuilt: %s\n", label);

    AGRemapCore::Z3Context::Impl& ctxImpl = AGRemapCore::getZ3ContextImplForTesting(ctxWrapper);
    z3::expr boolHandBuilt = handBuilt.is_bool() ? handBuilt : (handBuilt != ctxImpl.ctx->real_val(0));
    Z3Predicate original = AGRemapCore::makeZ3PredicateForTesting(std::make_unique<Z3Predicate::Impl>(boolHandBuilt, ctxImpl.ctx));

    std::string text;
    try {
        text = Z3IfPredGenerator::generate(original);
    } catch (const std::exception& e) {
        check(false, std::string(label) + ": generate() threw unexpectedly (" + e.what() + ")");
        return;
    }
    std::printf("    generated: %s\n", text.c_str());

    Z3Predicate reparsed = textToPredicate(text, ctxWrapper);
    check(provablyEquivalent(ctxWrapper, original, reparsed), std::string(label) + ": regenerated text is provably equivalent to the hand-built original");
}

void test_handBuiltExpressions() {
    Z3Context ctxWrapper;
    z3::context& ctx = *AGRemapCore::getZ3ContextImplForTesting(ctxWrapper).ctx;

    z3::expr a = ctx.real_const("$a");
    z3::expr b = ctx.real_const("$b");
    z3::expr c = ctx.real_const("$c");

    // Deliberately NOT ctx.bool_const("$flag") -- .ini has no static typing, so
    // IfPredZ3Generator's own "variable" leaf handler always builds a Real const, coerced to
    // Bool via '!= 0' wherever a boolean value is actually needed (coerceToBool). A raw
    // bool_const("$flag") free variable has no way to round-trip back through .ini text at all:
    // re-parsing "$flag" always reconstructs a *Real*-sorted "$flag", a completely different Z3
    // declaration from a Bool-sorted one of the same name -- not a bug, just the one inherent
    // representational limit of an untyped text format. Building 'flag'/'other' this way (a
    // Bool-sorted expression *derived from* a Real free variable) keeps the free variable's own
    // sort consistent with what a round trip actually reconstructs, so the equivalence check
    // below is checking something real.
    z3::expr flag = (a != ctx.real_val(0));
    z3::expr other = (b != ctx.real_val(0));

    z3::expr_vector distinctArgs(ctx);
    distinctArgs.push_back(a);
    distinctArgs.push_back(b);
    distinctArgs.push_back(c);
    checkHandBuilt(ctxWrapper, z3::distinct(distinctArgs), "n-ary distinct (3 args) expands to pairwise !=");
    checkHandBuilt(ctxWrapper, z3::implies(flag, other), "implies desugars to !a || b");
    checkHandBuilt(ctxWrapper, (flag != other), "xor-shaped boolean != between two bool consts");
    checkHandBuilt(ctxWrapper, -a, "bare unary minus of a variable rewrites to 0 - a");
    checkHandBuilt(ctxWrapper, a * (b + c), "multiply over an unflattened add needs a bracket");
    checkHandBuilt(ctxWrapper, ctx.real_val(1) / ctx.real_val(3), "a genuinely non-terminating rational falls back to (num / den)");
    checkHandBuilt(ctxWrapper, z3::ite(flag, ctx.real_val(1), ctx.real_val(0)), "ite(bool, 1, 0) unwraps back to the bool");
}

void test_unsupportedConstructsThrow() {
    std::printf("unsupportedConstructsThrow\n");

    Z3Context ctxWrapper;
    AGRemapCore::Z3Context::Impl& ctxImpl = AGRemapCore::getZ3ContextImplForTesting(ctxWrapper);
    z3::context& ctx = *ctxImpl.ctx;

    bool threwForGeneralIte = false;
    try {
        z3::expr generalIte = z3::ite(ctx.bool_const("$flag"), ctx.real_val(5), ctx.real_val(9));
        Z3Predicate p = AGRemapCore::makeZ3PredicateForTesting(std::make_unique<Z3Predicate::Impl>(generalIte != ctx.real_val(0), ctxImpl.ctx));
        Z3IfPredGenerator::generate(p);
    } catch (const std::invalid_argument&) {
        threwForGeneralIte = true;
    }
    check(threwForGeneralIte, "a general (non 0/1) ite throws std::invalid_argument");

    bool threwForBadName = false;
    try {
        z3::expr badName = ctx.real_const("notDollarPrefixed");
        Z3Predicate p = AGRemapCore::makeZ3PredicateForTesting(std::make_unique<Z3Predicate::Impl>(badName != ctx.real_val(0), ctxImpl.ctx));
        Z3IfPredGenerator::generate(p);
    } catch (const std::invalid_argument&) {
        threwForBadName = true;
    }
    check(threwForBadName, "a constant not shaped like '$name' throws std::invalid_argument");
}

} // namespace

int main() {
    // Unbuffered, so output survives even if the process fastfails/aborts partway through
    // (eg. an unmet z3++.h precondition assert) instead of being lost in a never-flushed buffer.
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    test_textRoundTrips();
    test_handBuiltExpressions();
    test_unsupportedConstructsThrow();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
        return 0;
    }

    std::printf("\n%d test(s) failed.\n", failures);
    return 1;
}
