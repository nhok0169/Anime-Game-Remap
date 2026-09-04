// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::Z3Predicate's pimpl member declaration order.
//
// Bug summary
// -----------
// Z3Predicate::Impl holds a z3::expr and a std::shared_ptr<z3::context> ('ctxKeepAlive') that keeps
// the expr's context alive. Members are destroyed in reverse declaration order, and the keep-alive
// used to be declared *after* the expr -- so for the last Z3Predicate alive on a given context,
// destroying it freed the z3::context first and then ran the z3::expr destructor, whose
// Z3_dec_ref read the just-freed context. That only faults when the freed block has already been
// reused, which produced two long-lived misdiagnoses:
//   * "destroying predicates of several already-gone contexts in an interleaved order crashes, but
//     grouped-by-context is fine" (documented as an unfixable Z3 limitation -- it was this), and
//   * nondeterministic access violations in ~IniFile() on real mods (IniFile's Z3Context member is
//     destroyed before its sections' IfPredParts, so "last owner is a predicate" is routine there).
//
// The fix is declaring 'ctxKeepAlive' before 'predicate' in Z3Internal.h. This test recreates the
// exact scenario the old note described: build predicates across many Z3Contexts, let every
// Z3Context wrapper die, shuffle the predicates, destroy them one by one with heap churn in
// between so that a dangling dec_ref lands on reused memory. With the old order this
// access-violates on the first round (confirmed); with the fix it runs every round clean.
// It exits non-zero on any failure and, being a crash test, the *process surviving* is the pass.
//
// Compile directly, e.g. (after `vcvarsall.bat x64`) -- same source list as Z3Predicate_test.cpp:
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <core>/src /I <extern>/ordered-map/include /I <extern>/utf8proc /I <cext>/z3/include ^
//      Z3Predicate_MemberOrder_test.cpp ^
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
//      /Fe:z3predicate_memberorder_test.exe ^
//      /link /LIBPATH:<cext>/z3/lib libz3.lib
//
// Copy libz3.dll next to the exe before running.
// -----------------------------------------------------------------------------

#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <random>
#include <string>
#include <vector>

using AGRemapCore::IfPredPart;
using AGRemapCore::IfPredPartType;
using AGRemapCore::Z3Context;
using AGRemapCore::Z3Predicate;

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    const int contexts = 20;
    const int predicatesPerContext = 3;
    const int rounds = 20;
    std::mt19937 rng(12345);

    for (int round = 0; round < rounds; ++round) {
        std::vector<std::unique_ptr<Z3Predicate>> predicates;

        for (int c = 0; c < contexts; ++c) {
            // The Z3Context wrapper dies at the end of this block: from here on only the
            // predicates' own keep-alives hold its z3::context.
            Z3Context ctx;
            for (int p = 0; p < predicatesPerContext; ++p) {
                std::string text = "if $x" + std::to_string(p) + " * " + std::to_string(p + 1) + " > " + std::to_string(c) + " && $x" + std::to_string(p) + " != 0";
                IfPredPart part(text, IfPredPartType::If, ctx);
                predicates.push_back(std::make_unique<Z3Predicate>(*part.query));
            }
        }

        std::shuffle(predicates.begin(), predicates.end(), rng);

        // Interleaved destruction with heap churn between each one -- the churn is what makes a
        // dangling dec_ref land on reused memory instead of silently reading a freed block.
        std::vector<std::string> churn;
        while (!predicates.empty()) {
            predicates.pop_back();
            churn.emplace_back(4096, 'x');
            if (churn.size() > 64) {
                churn.clear();
            }
        }

        std::printf("  [PASS] round %d: %d contexts x %d predicates destroyed interleaved\n", round + 1, contexts, predicatesPerContext);
    }

    std::printf("Z3Predicate member-order test: all %d rounds survived\n", rounds);
    return 0;
}
