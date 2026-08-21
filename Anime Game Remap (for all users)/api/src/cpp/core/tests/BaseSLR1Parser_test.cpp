// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::BaseSLR1Parser<Id, IdHash, IdEq> --
// the C++ port (in progress) of tools/parsing/BaseSLR1Parser.py. This pass only
// covers the miscellaneous setup (constructor/productions/startSymbol/clear/
// getNonTermSymbols) and getNullableSet -- getFirstSet/getFollowSet/constructDFA
// are ported separately.
//
// Test data is taken directly from the pure-Python original's own unit test
// (Testing/Unit Tester/UnitTester/Tests/test_BaseSLR1Parser.py), instantiated
// here with Id = int to mirror that test's list-based (index-keyed) production
// construction.
//
// This file has NO dependency on the project's build system (CMake/pybind11),
// Z3, utf8proc, ordered-map, or xxHash. Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /I <core>/include ^
//      BaseSLR1Parser_test.cpp <core>/src/tools/StringHash.cpp ^
//      /Fe:test.exe
// -----------------------------------------------------------------------------

#include "AGRemapCore/tools/parsing/BaseSLR1Parser.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/parsing/SyntaxErr.h"
#include "AGRemapCore/tools/parsing/Token.h"

#include <cstdio>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

using AGRemapCore::BaseSLR1Parser;
using AGRemapCore::ParseContext;
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

using Parser = BaseSLR1Parser<int>;
using Production = Parser::Production;

const std::string startToken = "TESTSTARTTOKEN";
const std::string endToken = "TESTENDTOKEN";
const std::string nullToken = "TESTNULLTOKEN";

std::vector<Production> basicProductions() {
    return {
        {"SPRIME", {startToken, "a", endToken}},
        {"S", {"S", "R", "S"}},
        {"S", {"a"}},
        {"S", {"b"}},
        {"R", {"+"}},
        {"R", {"-"}},
    };
}

Parser makeParser() {
    return Parser(basicProductions(), "SPRIME", startToken, endToken, nullToken, /*setup=*/false);
}

void test_productions_preserveInsertionOrder() {
    std::printf("test_productions_preserveInsertionOrder\n");

    // Productions is a tsl::ordered_map specifically so iteration order matches insertion order,
    // the same as the pure-Python original's real dict -- a plain std::unordered_map wouldn't make
    // this guarantee (its order depends on hash/bucket layout, not insertion sequence). This is
    // what lets constructDFA's traversal-order-dependent id generation actually line up with the
    // pure-Python original's, instead of being an unordered_map bucket-layout accident.
    Parser parser = makeParser();

    std::vector<int> idsInIterationOrder;
    for (const auto& [prodId, production] : parser.productions()) {
        idsInIterationOrder.push_back(prodId);
    }

    std::vector<int> expected = {0, 1, 2, 3, 4, 5};
    check(idsInIterationOrder == expected, "iterating productions() yields ids in insertion order, not hash-bucket order");
}

void test_constructor_and_productionsValidated() {
    std::printf("test_constructor_and_productionsValidated\n");

    Parser parser = makeParser();
    check(parser.productions().size() == 6, "constructor accepts a valid production list");

    // empty productions: valid, but startSymbol must still resolve -- exercised via setProductions directly
    bool emptyOk = true;
    try {
        parser.setProductions(std::vector<Production>{});
    } catch (...) {
        emptyOk = false;
    }
    check(emptyOk, "setProductions accepts an empty production list");

    // restore, then check a valid non-empty reassignment
    bool validOk = true;
    try {
        parser.setProductions(basicProductions());
    } catch (...) {
        validOk = false;
    }
    check(validOk, "setProductions accepts a valid non-empty production list");
    check(parser.productions().size() == 6, "setProductions replaced the production list");

    // a terminal symbol (startToken) on the LHS must be rejected
    bool threw = false;
    try {
        parser.setProductions(std::vector<Production>{
            {startToken, {nullToken}},
            {startToken, {"a", "S", "b"}},
        });
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "setProductions rejects a terminal symbol on the LHS of a production");
}

void test_startSymbol_isNonTerminal() {
    std::printf("test_startSymbol_isNonTerminal\n");

    Parser parser = makeParser();

    bool ok = true;
    try {
        parser.setStartSymbol("S");
    } catch (...) {
        ok = false;
    }
    check(ok, "setStartSymbol accepts a valid non-terminal");
    check(parser.startSymbol() == "S", "startSymbol reflects the new value");

    bool threw = false;
    try {
        parser.setStartSymbol("someTerminalSymbol");
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "setStartSymbol rejects a symbol that isn't a non-terminal");
}

void test_clear() {
    std::printf("test_clear\n");

    Parser parser = makeParser();
    parser.nullable["S"] = true;
    parser.first["S"] = {"a"};
    parser.follow["S"] = {"b"};

    parser.clear();

    check(parser.nullable.empty(), "clear empties nullable");
    check(parser.first.empty(), "clear empties first");
    check(parser.follow.empty(), "clear empties follow");
}

void test_getNonTermSymbols() {
    std::printf("test_getNonTermSymbols\n");

    Parser parser = makeParser();
    std::unordered_set<std::string> expected = {"SPRIME", "S", "R"};
    check(parser.getNonTermSymbols() == expected, "getNonTermSymbols finds every LHS symbol");
    check(parser.nonTermSymbols() == expected, "nonTermSymbols() cache matches getNonTermSymbols()");
}

// Shared fixture: same CFG used by both the Python original's
// test_differentCFG_getNullableSet and test_differentCFG_getFirstSet.
std::vector<Production> qrsProductions() {
    return {
        {"SPRIME", {startToken, "S", endToken}},
        {"S", {"c"}},
        {"S", {"Q", "R", "S"}},
        {"Q", {"R"}},
        {"Q", {"d"}},
        {"R", {nullToken}},
        {"R", {"b"}},
    };
}

void test_getNullableSet() {
    std::printf("test_getNullableSet\n");

    Parser parser(qrsProductions(), "SPRIME", startToken, endToken, nullToken, /*setup=*/false);

    std::unordered_map<std::string, bool> expected = {
        {"SPRIME", false},
        {"S", false},
        {"Q", true},
        {"R", true},
    };

    std::unordered_map<std::string, bool> result = parser.getNullableSet();
    check(result == expected, "getNullableSet matches the pure-Python original's expected result");
}

void test_getFirstSet() {
    std::printf("test_getFirstSet\n");

    Parser parser(qrsProductions(), "SPRIME", startToken, endToken, nullToken, /*setup=*/false);

    std::unordered_map<std::string, std::unordered_set<std::string>> expected = {
        {"SPRIME", {startToken}},
        {"S", {"b", "c", "d"}},
        {"Q", {"b", "d"}},
        {"R", {"b"}},
    };

    std::unordered_map<std::string, std::unordered_set<std::string>> result = parser.getFirstSet();
    check(result == expected, "getFirstSet matches the pure-Python original's expected result");
    check(parser.nullable.at("R") == true, "getFirstSet's default updateNullable=true refreshed nullable as a side effect");

    // getFirst itself, exercised directly against a fixed nullable/first (the shape getFollowSet
    // will reuse this helper with) -- prefix reading stops at the first non-nullable symbol
    std::unordered_map<std::string, bool> nullableSet = {{"Q", false}, {"R", true}};
    std::unordered_map<std::string, std::unordered_set<std::string>> firstSet = {{"Q", {"d"}}, {"R", {"b"}}};

    std::unordered_set<std::string> stopsAtNonNullable = parser.getFirst({"Q", "R"}, nullableSet, firstSet);
    check(stopsAtNonNullable == std::unordered_set<std::string>{"d"}, "getFirst stops after a non-nullable non-terminal");

    std::unordered_set<std::string> terminalShortCircuits = parser.getFirst({"x", "Q"}, nullableSet, firstSet);
    check(terminalShortCircuits == std::unordered_set<std::string>{"x"}, "getFirst returns immediately on a leading terminal");

    std::unordered_set<std::string> skipsEpsilon = parser.getFirst({nullToken, "R"}, nullableSet, firstSet);
    check(skipsEpsilon == std::unordered_set<std::string>{"b"}, "getFirst skips a leading nullToken symbol");
}

void test_getFollowSet() {
    std::printf("test_getFollowSet\n");

    {
        Parser parser(qrsProductions(), "SPRIME", startToken, endToken, nullToken, /*setup=*/false);

        std::unordered_map<std::string, std::unordered_set<std::string>> expected = {
            {"S", {endToken}},
            {"Q", {"b", "c", "d"}},
            {"R", {"b", "c", "d"}},
        };

        std::unordered_map<std::string, std::unordered_set<std::string>> result = parser.getFollowSet();
        check(result == expected, "getFollowSet matches the pure-Python original's expected result");
        check(result.find("SPRIME") == result.end(), "getFollowSet has no entry for a symbol that never appears on any RHS");
    }

    {
        // basicProductions()'s "S -> S R S" makes 'val' and 'prodKey' the same symbol on both the
        // first and last occurrence of S -- the self-aliasing case the copy-before-insert fix in
        // getFollowSet guards against. Note SPRIME's own rule is [startToken, "a", endToken] (a
        // literal terminal "a", not the nonterminal S -- matching the pure-Python fixture this was
        // copied from), so S is unreachable from SPRIME and every FOLLOW(S)/FOLLOW(R) contribution
        // comes only from "S -> S R S" itself. Expected values hand-derived from the grammar:
        //   FOLLOW(S) = {+, -} (from S -> S . R S) union FOLLOW(S) itself (from S -> S R S ., a no-op)
        //   FOLLOW(R) = FIRST(S) = {a, b} (from S -> S R . S)
        Parser parser = makeParser();

        std::unordered_map<std::string, std::unordered_set<std::string>> expected = {
            {"S", {"+", "-"}},
            {"R", {"a", "b"}},
        };

        std::unordered_map<std::string, std::unordered_set<std::string>> result = parser.getFollowSet();
        check(result == expected, "getFollowSet handles a self-recursive production (S -> S R S) without corrupting its own follow set");
    }
}

// Both grammars, and their expected {state count, accept count, reduction shape}, are taken
// directly from the pure-Python original's test_differentCFG_slr1DFAConstructed. Exact STATE ids
// aren't checked -- this port generates them from an unordered_map-keyed traversal (productions_,
// neighbours), so which state gets which generated id is order-dependent, unlike the pure-Python
// original's insertion-ordered dict. What IS checked, and is fully order-independent: total state
// count, accept-state count, and -- since a production's id is always just its source-list index
// (0,1,2,3), deterministic regardless of traversal order -- the exact multiset of production ids
// each reduction resolves to, keyed by lookahead where the grammar needs it.
struct ReductionShape {
    std::multiset<int> unconditional;                                   // states with exactly 1 item -> prodInd
    std::vector<std::unordered_map<std::string, int>> conditional;      // states with >1 item -> {followSymbol: prodInd}
};

ReductionShape summarizeReductions(const Parser::Reductions& reductions) {
    ReductionShape shape;
    for (const auto& [stateId, reduction] : reductions) {
        if (std::holds_alternative<int>(reduction)) {
            shape.unconditional.insert(std::get<int>(reduction));
        } else {
            shape.conditional.push_back(std::get<std::unordered_map<std::string, int>>(reduction));
        }
    }
    return shape;
}

void test_constructDFA() {
    std::printf("test_constructDFA\n");

    {
        // LR(0): every accept state happens to reduce unconditionally (no dict entries)
        std::vector<Production> productions = {
            {"SPRIME", {startToken, "S", endToken}},
            {"S", {"S", "+", "T"}},
            {"S", {"T"}},
            {"T", {"d"}},
        };
        Parser parser(productions, "SPRIME", startToken, endToken, nullToken, /*setup=*/false);

        Parser::States states = parser.constructDFA();

        check(states.size() == 8, "LR(0) grammar produces 8 states");
        check(parser.dfa().stateSize() == 8, "LR(0) grammar's DFA has 8 states");
        check(parser.dfa().acceptSize() == 4, "LR(0) grammar's DFA has 4 accept states");
        check(parser.reductions().size() == 4, "LR(0) grammar has 4 reduction entries");

        ReductionShape shape = summarizeReductions(parser.reductions());
        check(shape.conditional.empty(), "LR(0) grammar's reductions are all unconditional");
        check(shape.unconditional == std::multiset<int>{0, 1, 2, 3}, "LR(0) grammar reduces via every production exactly once");
    }

    {
        // SLR(1): needs FOLLOW-set disambiguation -- state reducing "S -> T ." only on endToken
        std::vector<Production> productions = {
            {"SPRIME", {startToken, "S", endToken}},
            {"S", {"T", "+", "S"}},
            {"S", {"T"}},
            {"T", {"d"}},
        };
        Parser parser(productions, "SPRIME", startToken, endToken, nullToken, /*setup=*/false);

        Parser::States states = parser.constructDFA();

        check(states.size() == 8, "SLR(1) grammar produces 8 states");
        check(parser.dfa().stateSize() == 8, "SLR(1) grammar's DFA has 8 states");
        check(parser.dfa().acceptSize() == 4, "SLR(1) grammar's DFA has 4 accept states");
        check(parser.reductions().size() == 4, "SLR(1) grammar has 4 reduction entries");

        ReductionShape shape = summarizeReductions(parser.reductions());
        check(shape.unconditional == std::multiset<int>{0, 1, 3}, "SLR(1) grammar's 3 unambiguous states reduce via prods 0, 1, 3");
        check(shape.conditional.size() == 1, "SLR(1) grammar has exactly 1 lookahead-disambiguated state");

        if (shape.conditional.size() == 1) {
            std::unordered_map<std::string, int> expectedConditional = {{endToken, 2}};
            check(shape.conditional[0] == expectedConditional, "the disambiguated state reduces via prod 2 only on endToken");
        }
    }

    {
        // setup() should wire nullable/first/follow/DFA together the same way the constructor's
        // setup=true path does (both go through the same method).
        std::vector<Production> productions = {
            {"SPRIME", {startToken, "S", endToken}},
            {"S", {"S", "+", "T"}},
            {"S", {"T"}},
            {"T", {"d"}},
        };
        Parser parser(productions, "SPRIME", startToken, endToken, nullToken, /*setup=*/true);

        check(!parser.nullable.empty(), "setup() (via the constructor) populated nullable");
        check(!parser.first.empty(), "setup() (via the constructor) populated first");
        check(!parser.follow.empty(), "setup() (via the constructor) populated follow");
        check(parser.dfa().stateSize() == 8, "setup() (via the constructor) constructed the DFA");
        check(parser.reductions().size() == 4, "setup() (via the constructor) populated reductions");
    }
}

void checkToken(const Token& actual, const std::string& expectedType, const std::string& expectedVal,
                 std::size_t expectedLineNo, std::size_t expectedCharNo, const char* description) {
    bool ok = actual.type.has_value() && *actual.type == expectedType && actual.val == expectedVal
              && actual.lineNo == expectedLineNo && actual.charNo == expectedCharNo;
    check(ok, description);
}

void test_parse() {
    std::printf("test_parse\n");

    {
        // Same fixture as the pure-Python original's test_differentCFGAndText_parseTreeGenerated
        // (its "abcdef" case): S -> A c B, A -> ab | ff, B -> def | ef. Unlike every other test in
        // this file, this checks the EXACT node ids the pure-Python original's test hardcodes (not
        // just tree shape) -- deliberately: parse()'s node-id generation is a single deterministic
        // linear pass over the tokens (no unordered/order-dependent traversal involved at all, unlike
        // constructDFA's state ids), so with the default IncIdGenerator<int> starting at 1 -- the
        // same start/step the pure-Python original's own mocked _generateNodeId counter uses -- the
        // exact sequence of assigned ids is reproducible and should match byte-for-byte.
        std::vector<Production> productions = {
            {"SPRIME", {startToken, "S", endToken}}, // 0
            {"S", {"A", "c", "B"}},                  // 1
            {"A", {"a", "b"}},                       // 2
            {"A", {"f", "f"}},                       // 3
            {"B", {"d", "e", "f"}},                  // 4
            {"B", {"e", "f"}},                       // 5
        };
        Parser parser(productions, "SPRIME", startToken, endToken, nullToken, /*setup=*/true);

        ParseContext ctx("abcdef", std::string("MyFile.agr"), 8);
        std::vector<Token> tokens = {
            Token("a", "a", 8, 1), Token("b", "b", 8, 2), Token("c", "c", 8, 3),
            Token("d", "d", 8, 4), Token("e", "e", 8, 5), Token("f", "f", 8, 6),
        };

        Parser::Tree tree = parser.parse(tokens, &ctx);

        check(tree.rootId() == 12, "parse produces the exact same root id as the pure-Python original");
        check(tree.nodes.size() == 12, "parse produces exactly 12 nodes");

        auto checkLeaf = [&](int id, const std::string& type, const std::string& val, std::size_t lineNo, std::size_t charNo, const char* desc) {
            const auto* node = tree.getNode(id, false);
            if (check(node != nullptr, desc)) {
                check(!node->prodId.has_value(), desc);
                if (check(node->token.has_value(), desc)) {
                    checkToken(*node->token, type, val, lineNo, charNo, desc);
                }
            }
        };
        auto checkReduceNode = [&](int id, int expectedProdId, const char* desc) {
            const auto* node = tree.getNode(id, false);
            if (check(node != nullptr, desc)) {
                check(!node->token.has_value(), desc);
                check(node->prodId.has_value() && *node->prodId == expectedProdId, desc);
            }
        };

        checkLeaf(1, startToken, startToken, 8, 0, "node 1 is the shifted start token");
        checkLeaf(2, "a", "a", 8, 1, "node 2 is the shifted 'a'");
        checkLeaf(3, "b", "b", 8, 2, "node 3 is the shifted 'b'");
        checkReduceNode(4, 2, "node 4 reduces A -> ab (prod 2)");
        checkLeaf(5, "c", "c", 8, 3, "node 5 is the shifted 'c'");
        checkLeaf(6, "d", "d", 8, 4, "node 6 is the shifted 'd'");
        checkLeaf(7, "e", "e", 8, 5, "node 7 is the shifted 'e'");
        checkLeaf(8, "f", "f", 8, 6, "node 8 is the shifted 'f'");
        checkReduceNode(9, 4, "node 9 reduces B -> def (prod 4)");
        checkReduceNode(10, 1, "node 10 reduces S -> AcB (prod 1)");
        checkLeaf(11, endToken, endToken, 9, 0, "node 11 is the shifted end token");
        checkReduceNode(12, 0, "node 12 (root) reduces SPRIME -> start S end (prod 0)");

        Parser::Tree::Children expectedChildren = {
            {4, {2, 3}}, {9, {6, 7, 8}}, {10, {4, 5, 9}}, {12, {1, 10, 11}},
        };
        check(tree.children == expectedChildren, "parse produces the exact same children map as the pure-Python original");
    }

    {
        // The pure-Python original's LR(0) SyntaxErr case: "d+d+d-d+d+d" fails exactly at the '-'
        // (position 6) since the grammar (S -> S+T | T, T -> d) has no production for '-'.
        std::vector<Production> productions = {
            {"SPRIME", {startToken, "S", endToken}},
            {"S", {"S", "+", "T"}},
            {"S", {"T"}},
            {"T", {"d"}},
        };
        Parser parser(productions, "SPRIME", startToken, endToken, nullToken, /*setup=*/true);

        ParseContext ctx("d+d+d-d+d+d", std::string("MyFile.agr"), 8);
        std::vector<Token> tokens;
        std::string text = "d+d+d-d+d+d";
        for (std::size_t i = 0; i < text.size(); ++i) {
            std::string ch(1, text[i]);
            tokens.emplace_back(ch, ch, 8, i + 1);
        }

        bool threw = false;
        try {
            parser.parse(tokens, &ctx);
        } catch (const SyntaxErr& e) {
            threw = true;
            checkToken(e.token(), "-", "-", 8, 6, "the SyntaxErr's token is the offending '-' at the exact position the pure-Python original expects");
        }
        check(threw, "an input with no valid continuation raises SyntaxErr");
    }
}

void test_twoStartProductionsSharingFirstSymbol() {
    // Diagnostic: SympyParser/IfPredParser's real grammars have TWO productions for the start
    // symbol ("start" and "start empty"), both beginning with startToken -- a shape none of the
    // other tests in this file happen to exercise (they all use a single start production).
    std::printf("test_twoStartProductionsSharingFirstSymbol\n");

    std::vector<Production> productions = {
        {"SPRIME", {startToken, "S", endToken}}, // 0: "start"-shaped
        {"SPRIME", {startToken, endToken}},      // 1: "start empty"-shaped
        {"S", {"a"}},                            // 2
    };
    Parser parser(productions, "SPRIME", startToken, endToken, nullToken, /*setup=*/true);

    check(parser.dfa().stateSize() > 1, "two-start-production grammar builds a non-trivial DFA");

    bool threwNonEmpty = false;
    try {
        parser.parse({Token("a", "a", 1, 1)});
    } catch (const SyntaxErr&) {
        threwNonEmpty = true;
    }
    check(!threwNonEmpty, "parses 'a' (start -> START S END, S -> a) without a SyntaxErr");

    bool threwEmpty = false;
    try {
        parser.parse({});
    } catch (const SyntaxErr&) {
        threwEmpty = true;
    }
    check(!threwEmpty, "parses empty input (start empty -> START END) without a SyntaxErr");
}

void test_ifPredGrammarShapeWithIntIds() {
    // Diagnostic: reproduce IfPredParser's exact grammar shape/size but with Id=int (vector
    // constructor) to isolate whether a real-size grammar itself is the problem, or something
    // specific to Id=std::string / the UuidIdGenerator combination.
    std::printf("test_ifPredGrammarShapeWithIntIds\n");

    std::vector<Production> productions = {
        {"pred_prime",  {startToken, "pred", endToken}},
        {"pred_prime",  {startToken, endToken}},
        {"pred",        {"test"}},
        {"pred",        {"pred", "AND", "test"}},
        {"pred",        {"pred", "OR", "test"}},
        {"test",        {"keyexpr"}},
        {"test",        {"keyexpr", "EQ", "keyexpr"}},
        {"test",        {"keyexpr", "NE", "keyexpr"}},
        {"test",        {"keyexpr", "GT", "keyexpr"}},
        {"test",        {"keyexpr", "GE", "keyexpr"}},
        {"test",        {"keyexpr", "LT", "keyexpr"}},
        {"test",        {"keyexpr", "LE", "keyexpr"}},
        {"keyexpr",     {"addexpr"}},
        {"keyexpr",     {"NULL"}},
        {"addexpr",     {"multexpr"}},
        {"addexpr",     {"addexpr", "PLUS", "multexpr"}},
        {"addexpr",     {"addexpr", "MINUS", "multexpr"}},
        {"multexpr",    {"nterm"}},
        {"multexpr",    {"multexpr", "STAR", "nterm"}},
        {"multexpr",    {"multexpr", "SLASH", "nterm"}},
        {"nterm",       {"term"}},
        {"nterm",       {"NOT", "nterm"}},
        {"term",        {"ID"}},
        {"term",        {"INT"}},
        {"term",        {"FLOAT"}},
        {"term",        {"LPAREN", "pred", "RPAREN"}},
    };
    Parser parser(productions, "pred_prime", startToken, endToken, nullToken, /*setup=*/true);

    check(parser.dfa().stateSize() > 1, "IfPredParser-shaped grammar (int ids) builds a non-trivial DFA");

    bool threw = false;
    try {
        parser.parse({Token("ID", "x", 1, 1)});
    } catch (const SyntaxErr&) {
        threw = true;
    }
    check(!threw, "IfPredParser-shaped grammar (int ids) parses a single ID without a SyntaxErr");
}

} // namespace

int main() {
    test_ifPredGrammarShapeWithIntIds();
    test_twoStartProductionsSharingFirstSymbol();
    test_productions_preserveInsertionOrder();
    test_constructor_and_productionsValidated();
    test_startSymbol_isNonTerminal();
    test_clear();
    test_getNonTermSymbols();
    test_getNullableSet();
    test_getFirstSet();
    test_getFollowSet();
    test_constructDFA();
    test_parse();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
        return 0;
    }

    std::printf("\n%d test(s) failed.\n", failures);
    return 1;
}
