// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::ParseTree<Id, IdHash, IdEq> and its
// AGRemapCore::ParseNode<Id> / AGRemapCore::Node<Id> dependencies -- the C++ port
// of tools/parsing/ParseTree.py + tools/nodes/ParseNode.py + tools/nodes/Node.py.
//
// The tree fixture is taken directly from the pure-Python original's own unit
// test (test_BaseSLR1Parser.py's test_differentCFGAndText_parseTreeGenerated,
// the "abcdef" grammar's expected tree), instantiated here with Id = int to
// match that test's own int-keyed nodes/children/prodIds.
//
// This file has NO dependency on the project's build system (CMake/pybind11),
// Z3, utf8proc, ordered-map, or xxHash. Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /I <core>/include ^
//      ParseTree_test.cpp <core>/src/tools/parsing/Token.cpp ^
//      /Fe:test.exe
// -----------------------------------------------------------------------------

#include "AGRemapCore/tools/parsing/ParseTree.h"

#include <cstdio>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using AGRemapCore::ParseNode;
using AGRemapCore::ParseTree;
using AGRemapCore::Token;

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (condition) {
        std::printf("  [PASS] %s\n", description);
    } else {
        std::printf("  [FAIL] %s\n", description);
        ++failures;
    }
}

using Tree = ParseTree<int>;

Tree makeAbcdefTree() {
    Tree::Nodes nodes;
    nodes.emplace(1, ParseNode<int>(1, std::nullopt, Token("STARTTOKEN", "STARTTOKEN", 8, 0)));
    nodes.emplace(2, ParseNode<int>(2, std::nullopt, Token("a", "a", 8, 1)));
    nodes.emplace(3, ParseNode<int>(3, std::nullopt, Token("b", "b", 8, 2)));
    nodes.emplace(4, ParseNode<int>(4, 2));
    nodes.emplace(5, ParseNode<int>(5, std::nullopt, Token("c", "c", 8, 3)));
    nodes.emplace(6, ParseNode<int>(6, std::nullopt, Token("d", "d", 8, 4)));
    nodes.emplace(7, ParseNode<int>(7, std::nullopt, Token("e", "e", 8, 5)));
    nodes.emplace(8, ParseNode<int>(8, std::nullopt, Token("f", "f", 8, 6)));
    nodes.emplace(9, ParseNode<int>(9, 4));
    nodes.emplace(10, ParseNode<int>(10, 1));
    nodes.emplace(11, ParseNode<int>(11, std::nullopt, Token("ENDTOKEN", "ENDTOKEN", 9, 0)));
    nodes.emplace(12, ParseNode<int>(12, 0));

    Tree::Children children;
    children.emplace(4, std::vector<int>{2, 3});
    children.emplace(9, std::vector<int>{6, 7, 8});
    children.emplace(10, std::vector<int>{4, 5, 9});
    children.emplace(12, std::vector<int>{1, 10, 11});

    return Tree(std::move(nodes), std::move(children), 12);
}

void test_construction_and_rootId() {
    std::printf("test_construction_and_rootId\n");

    Tree tree = makeAbcdefTree();
    check(tree.rootId() == 12, "constructor sets rootId");
    check(tree.nodes.size() == 12, "constructor sets nodes");
    check(tree.children.size() == 4, "constructor sets children");

    bool threw = false;
    try {
        Tree bad(Tree::Nodes{}, Tree::Children{}, 999);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "constructor rejects a rootId not present in nodes");

    bool setOk = true;
    try {
        tree.setRootId(10);
    } catch (...) {
        setOk = false;
    }
    check(setOk, "setRootId accepts an id that exists in nodes");
    check(tree.rootId() == 10, "setRootId updated rootId");

    bool setThrew = false;
    try {
        tree.setRootId(999);
    } catch (const std::invalid_argument&) {
        setThrew = true;
    }
    check(setThrew, "setRootId rejects an id not present in nodes");
    check(tree.rootId() == 10, "a rejected setRootId leaves the previous rootId untouched");
}

void test_getNode() {
    std::printf("test_getNode\n");

    Tree tree = makeAbcdefTree();

    const ParseNode<int>* leaf = tree.getNode(2);
    check(leaf != nullptr, "getNode finds an existing leaf/token node");
    check(leaf->id() == 2, "found node has the expected id");
    check(!leaf->prodId.has_value(), "a token-shift node has no prodId");
    check(leaf->token.has_value() && leaf->token->val == "a", "a token-shift node carries its token");

    const ParseNode<int>* reduceNode = tree.getNode(4);
    check(reduceNode != nullptr, "getNode finds an existing reduce node");
    check(reduceNode->prodId.has_value() && *reduceNode->prodId == 2, "a reduce node carries its prodId");
    check(!reduceNode->token.has_value(), "a reduce node has no token");

    const ParseNode<int>* missing = tree.getNode(999, /*errorOnNotFound=*/false);
    check(missing == nullptr, "getNode returns nullptr for a missing id when errorOnNotFound=false");

    bool threw = false;
    try {
        tree.getNode(999);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    check(threw, "getNode throws for a missing id when errorOnNotFound=true (the default)");
}

void test_isChild() {
    std::printf("test_isChild\n");

    Tree tree = makeAbcdefTree();

    check(tree.isChild(2), "a leaf/token node (no entry in children) is a 'child' (leaf)");
    check(!tree.isChild(4), "an internal reduce node (has an entry in children) is not a 'child' (leaf)");
    check(!tree.isChild(999), "a nonexistent node id is not a 'child'");
}

} // namespace

int main() {
    test_construction_and_rootId();
    test_getNode();
    test_isChild();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
        return 0;
    }

    std::printf("\n%d test(s) failed.\n", failures);
    return 1;
}
