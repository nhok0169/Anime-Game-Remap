// -----------------------------------------------------------------------------
// Standalone regression test for BaseAhoCorasickDFA<TrieVal>::getKVP(...) and
// BaseAhoCorasickDFA<TrieVal>::getMaximal(...)
//
// Bug summary
// -----------
// Both methods are declared to return std::tuple<const std::string&, const TrieVal&>
// (a tuple OF REFERENCES). The old implementation built the return value with
// std::make_tuple(*keywordPtr, *valPtr), which decays its arguments and
// constructs a BY-VALUE std::tuple<std::string, TrieVal> temporary. That
// temporary then gets implicitly converted to the declared reference-tuple type,
// binding the reference members to elements of the temporary -- which is
// destroyed at the end of the return statement. The tuple actually handed back
// to the caller holds dangling references (use-after-free-of-a-temporary UB).
//
// The fix replaces std::make_tuple(...) with std::tie(...), which builds the
// tuple-of-references directly with no intermediate by-value temporary.
//
// Why an immediate structured-binding read can mask this bug
// ------------------------------------------------------------
// `auto [k, v] = dfa.getMaximal(txt);` often "works" by pure luck: the compiler
// may not yet have reused the temporary's stack slot for anything else by the
// time `k`/`v` are read. This test deliberately avoids that trap: it keeps the
// returned tuple alive across a call to an unrelated function that measurably
// disturbs the stack (recursion + local array writes) before reading the
// tuple's elements, and repeats the whole thing many times with varying
// disturbance sizes to avoid false negatives from allocator/stack reuse luck.
//
// This file has NO dependency on the project's build system (CMake/pybind11)
// or on Z3 -- it only needs AGRemapCore's header-only trie code plus utf8proc
// (used transitively for grapheme iteration). It is meant to be compiled
// directly, e.g.:
//
//   cl /std:c++23 /EHsc /I <core>/include /I <utf8proc_src> ^
//      BaseAhoCorasickDFA_ReferenceLifetime_test.cpp ^
//      <utf8proc_src>/utf8proc.c <core>/src/tools/StringTools.cpp ^
//      <core>/src/tools/StringHash.cpp ^
//      <core>/src/tools/grapheme/GraphemeIterator.cpp ^
//      <core>/src/tools/grapheme/GraphemeRange.cpp ^
//      /Fe:test.exe
//
// (g++/clang++ equivalent: swap /std:c++23 /EHsc /I /Fe: for
//  -std=c++23 -I ... -o test.exe)
// -----------------------------------------------------------------------------

#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <tuple>
#include <unordered_set>

using AGRemapCore::BaseAhoCorasickDFA;

namespace {

// Recurses and writes to a stack-local buffer at every level so the memory
// the dangling references used to point at gets overwritten with recognizable
// garbage (0xCD) rather than happening to still hold the old bytes.
void disturbStack(int depth, int fillByte) {
    volatile char buf[256];
    std::memset(const_cast<char *>(buf), fillByte, sizeof(buf));
    if (depth > 0) {
        disturbStack(depth - 1, fillByte ^ 0x5A);
    }
    // Touch buf again after the recursive call so it isn't optimized away.
    buf[0] = static_cast<char>(buf[0] + 1);
}

int g_failures = 0;

void expect(bool cond, const char *what) {
    if (!cond) {
        std::printf("  [FAIL] %s\n", what);
        g_failures++;
    } else {
        std::printf("  [ OK ] %s\n", what);
    }
}

void testGetKVP() {
    std::printf("-- getKVP --\n");
    BaseAhoCorasickDFA<std::unordered_set<int>> dfa;
    dfa.add(std::string("hello"), std::unordered_set<int>{1, 2, 3});

    // Capture the whole tuple (references included) rather than reading
    // immediately via structured bindings -- this is the part a naive test
    // ("auto [k, v] = dfa.getKVP(...); use k, v right away") would skip,
    // and the part that let this bug hide before.
    std::tuple<const std::string &, const std::unordered_set<int> &> result =
        dfa.getKVP(std::string_view("hello"));

    // Disturb the stack repeatedly, with varying depth/fill patterns, between
    // obtaining the tuple and reading it, to make a dangling reference show
    // up reliably instead of by luck.
    for (int i = 0; i < 20; i++) {
        disturbStack(50 + i, 0xCD + i);
    }

    const std::string &keyword = std::get<0>(result);
    const std::unordered_set<int> &val = std::get<1>(result);

    expect(keyword == "hello", "getKVP: keyword still reads \"hello\" after stack disturbance");
    expect(val.size() == 3 && val.count(1) && val.count(2) && val.count(3),
           "getKVP: value set still reads {1,2,3} after stack disturbance");
}

void testGetMaximal() {
    std::printf("-- getMaximal --\n");
    BaseAhoCorasickDFA<std::unordered_set<int>> dfa;
    dfa.add(std::string("he"), std::unordered_set<int>{10});
    dfa.add(std::string("hello"), std::unordered_set<int>{20, 21});

    std::tuple<const std::string &, const std::unordered_set<int> &> result =
        dfa.getMaximal(std::string_view("hello"));

    for (int i = 0; i < 20; i++) {
        disturbStack(50 + i, 0x11 + i);
    }

    const std::string &keyword = std::get<0>(result);
    const std::unordered_set<int> &val = std::get<1>(result);

    expect(keyword == "hello", "getMaximal: keyword still reads \"hello\" (the maximal match) after stack disturbance");
    expect(val.size() == 2 && val.count(20) && val.count(21),
           "getMaximal: value set still reads {20,21} after stack disturbance");
}

} // namespace

int main() {
    testGetKVP();
    testGetMaximal();

    if (g_failures == 0) {
        std::printf("\nAll checks passed (%d failures).\n", g_failures);
        return 0;
    }

    std::printf("\n%d check(s) FAILED -- dangling reference regression detected.\n", g_failures);
    return 1;
}
