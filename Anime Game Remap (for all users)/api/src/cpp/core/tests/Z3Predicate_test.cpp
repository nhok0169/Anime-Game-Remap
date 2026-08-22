// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::Z3Predicate's boolean-combination surface
// (operator&/operator|/operator!/simplify/isSatisfiable/sameContext/belongsTo/trueValue/
// falseValue) -- added alongside the Ini Graph Editing subsystem's migration onto Z3Predicate
// (IniSectionGraph.py/ResGroupCollect.py), which is this API's first real consumer.
//
// The correctness check for &/|/! is the same real z3::solver-based provable-equivalence pattern
// Z3IfPredGenerator_test.cpp/IfPredPart_test.cpp already use, not string comparison.
//
// Compile directly, e.g. (after `vcvarsall.bat x64`) -- same shape as IfPredPart_test.cpp's own
// header comment:
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <core>/src /I <extern>/ordered-map/include /I <extern>/utf8proc /I <cext>/z3/include ^
//      Z3Predicate_test.cpp ^
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
//      /Fe:z3predicate_test.exe ^
//      /link /LIBPATH:<cext>/z3/lib libz3.lib
// -----------------------------------------------------------------------------

#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"

// Manual verification only -- reaches past the opaque wrappers to build a solver directly for
// semantic-equivalence checks, same test-only hooks the sibling core/tests/ files use.
#include "tools/z3/Z3Internal.h"

#include <cstdio>
#include <memory>
#include <string>

using AGRemapCore::IfPredPart;
using AGRemapCore::IfPredPartType;
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

bool provablyEquivalent(const Z3Predicate& p1, const Z3Predicate& p2) {
    z3::context& ctx = AGRemapCore::getZ3PredicateImplForTesting(p1).predicate.ctx();
    z3::solver solver(ctx);
    solver.add(AGRemapCore::getZ3PredicateImplForTesting(p1).predicate != AGRemapCore::getZ3PredicateImplForTesting(p2).predicate);
    return solver.check() == z3::unsat;
}

Z3Predicate parse(const std::string& src, Z3Context& ctx) {
    ParseContext parseCtx(src);
    IfPredPart part("if " + src + " then", IfPredPartType::If, ctx, &parseCtx);
    return *part.query;
}

void test_andOrNot() {
    std::printf("andOrNot\n");

    Z3Context ctx;
    Z3Predicate a = parse("$a == 1", ctx);
    Z3Predicate b = parse("$b == 2", ctx);

    Z3Predicate anded = a & b;
    Z3Predicate expectedAnd = parse("$a == 1 && $b == 2", ctx);
    check(provablyEquivalent(anded, expectedAnd), "a & b is provably '$a == 1 && $b == 2'");

    Z3Predicate ored = a | b;
    Z3Predicate expectedOr = parse("$a == 1 || $b == 2", ctx);
    check(provablyEquivalent(ored, expectedOr), "a | b is provably '$a == 1 || $b == 2'");

    Z3Predicate notted = !a;
    Z3Predicate expectedNot = parse("$a != 1", ctx);
    check(provablyEquivalent(notted, expectedNot), "!a is provably '$a != 1'");
}

void test_simplify() {
    std::printf("simplify\n");

    Z3Context ctx;
    Z3Predicate redundant = parse("$a == 1 && $a == 1", ctx);
    Z3Predicate simplified = redundant.simplify();

    check(provablyEquivalent(redundant, simplified), "simplify() preserves logical equivalence");
}

void test_isSatisfiable() {
    std::printf("isSatisfiable\n");

    Z3Context ctx;
    check(parse("$a == 1", ctx).isSatisfiable(), "'$a == 1' is satisfiable");
    check(!parse("$a == 1 && $a == 2", ctx).isSatisfiable(), "'$a == 1 && $a == 2' is unsatisfiable");

    // The whole reason Z3 is a strictly better fit here than the old sympy
    // `satisfiable(..., use_lra_theory=True)` call was -- '!=' needs no special rewriting to
    // stay decidable, unlike the sympy version's own 'Ne -> Or(Lt, Gt)' workaround.
    check(parse("$a != 1", ctx).isSatisfiable(), "'$a != 1' is satisfiable (no Ne-rewrite workaround needed)");
    check(!(parse("$a == 1", ctx) & parse("$a != 1", ctx)).isSatisfiable(), "'$a == 1 && $a != 1' is unsatisfiable");
}

void test_sameContextAndBelongsTo() {
    std::printf("sameContextAndBelongsTo\n");

    Z3Context ctxA;
    Z3Context ctxB;

    Z3Predicate a1 = parse("$a == 1", ctxA);
    Z3Predicate a2 = parse("$b == 2", ctxA);
    Z3Predicate b1 = parse("$a == 1", ctxB);

    check(a1.sameContext(a2), "two predicates from the same Z3Context: sameContext() is true");
    check(!a1.sameContext(b1), "two predicates from different Z3Contexts: sameContext() is false");

    check(a1.belongsTo(ctxA), "a1 belongsTo its own context");
    check(!a1.belongsTo(ctxB), "a1 does not belongTo an unrelated context");
    check(b1.belongsTo(ctxB), "b1 belongsTo its own context");
}

void test_trueFalseValue() {
    std::printf("trueFalseValue\n");

    Z3Context ctx;
    Z3Predicate t = Z3Predicate::trueValue(ctx);
    Z3Predicate f = Z3Predicate::falseValue(ctx);

    check(t.isSatisfiable(), "trueValue() is satisfiable");
    check(!f.isSatisfiable(), "falseValue() is unsatisfiable");
    check(provablyEquivalent(!t, f), "!trueValue() is provably falseValue()");
    check(t.belongsTo(ctx), "trueValue(ctx) belongsTo ctx");

    // Combining a real predicate with the identity element should leave it unchanged --
    // this is exactly the fold IniSectionGraph._getQuery relies on for an empty queryPath.
    Z3Predicate a = parse("$a == 1", ctx);
    check(provablyEquivalent(a & t, a), "a & trueValue() is provably 'a' (AND identity)");
}

void test_reparent() {
    std::printf("reparent\n");

    Z3Context srcCtx;
    Z3Context dstCtx;

    Z3Predicate original = parse("$a == 1 && $b != 2", srcCtx);
    check(!original.belongsTo(dstCtx), "sanity: original does not already belong to dstCtx");

    std::optional<Z3Predicate> reparented = IfPredPart::reparent(original, dstCtx);
    check(reparented.has_value(), "reparent() succeeds for a representable predicate");
    if (reparented.has_value()) {
        check(reparented->belongsTo(dstCtx), "reparented predicate belongsTo the target context");
        check(!reparented->sameContext(original), "reparented predicate does not share the original's context");

        // Prove equivalence indirectly, since the two predicates can't be compared by a single
        // solver at all (mismatched contexts) -- reparent the original's own text back through
        // dstCtx a second, independent way and confirm the two dstCtx-side predicates agree.
        Z3Predicate expectedInDst = parse("$a == 1 && $b != 2", dstCtx);
        check(provablyEquivalent(*reparented, expectedInDst), "reparented predicate is provably equivalent, re-expressed in the target context");
    }

    // A true/false literal (the IniSectionGraph 'empty queryPath' identity element) round-trips
    // too -- this is the concrete case ResGroupCollect.py's cross-graph query combination relies on.
    Z3Predicate trueOrig = Z3Predicate::trueValue(srcCtx);
    std::optional<Z3Predicate> trueReparented = IfPredPart::reparent(trueOrig, dstCtx);
    check(trueReparented.has_value() && trueReparented->belongsTo(dstCtx) && trueReparented->isSatisfiable(), "trueValue() reparents cleanly into a different context");
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    test_andOrNot();
    test_simplify();
    test_isSatisfiable();
    test_sameContextAndBelongsTo();
    test_trueFalseValue();
    test_reparent();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
        return 0;
    }

    std::printf("\n%d test(s) failed.\n", failures);
    return 1;
}
