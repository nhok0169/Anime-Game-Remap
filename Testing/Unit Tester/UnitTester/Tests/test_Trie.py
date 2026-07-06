import sys
from functools import reduce
import unittest.mock as mock
from .baseTrieTest import BasePyTrieTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class TrieTest(BasePyTrieTest):

    # ============= __getitem__ ======================

    @mock.patch("src.py.FixRaidenBoss2.Trie.get")
    def test_getItemFromTrie_calledTrieGet(self, m_get):
        tests = [["shappy"],
                 ["s"],
                 ["pls"],
                 ["apple"],
                 ["le"],
                 ["shappyer"],
                 ["aaa"],
                 [""],
                 ["bbb"]]
        
        for test in tests:
            keyword = test[0]
            self._trie[keyword]
            m_get.assert_called_with(keyword)

    # ================================================
    # ============= __setitem__ ======================

    @mock.patch("src.py.FixRaidenBoss2.Trie.add")
    def test_setItemForTrie_referencedTrieAdd(self, m_add):
        data = [["boooo", 0, 5, True],
                ["", [], 0, True],
                ["boot", FRB.Algo(), 1, True],
                ["boot", "my new boot", 0, False],
                ["shoelace", None, 8, True],
                ["high heels", {}, 10, True],
                ["😆😄", set(), 2, True],
                ["high", 9.7, 0, True]]
        
        dataLen = len(data)

        for i in range(dataLen):
            kvpData = data[i]
            keyword = kvpData[0]
            value = kvpData[1] 

            self._trie[keyword] = value
            m_add.assert_called_with(keyword, value)

    # ================================================
    # ============= __contains__ =====================

    def test_differentKeywords_keywordInTrie(self):
        tests = [["shappy", True],
                 ["s", True],
                 ["pls", True],
                 ["apple", True],
                 ["le", True],
                 ["shappyer", True],
                 ["aaa", False],
                 ["", False],
                 ["bbb", False]]
        
        for test in tests:
            keyword = test[0]
            expected = test[1]

            result = keyword in self._trie
            self.assertEqual(result, expected)

    # ================================================
    # ================ clear =========================

    @mock.patch("src.py.FixRaidenBoss2.Trie.clearCache")
    def test_trieWithData_trieDataCleared(self, m_clearCache):
        self._trie.clear()

        self.compareDict(self._trie._children, {})
        self.compareDict(self._trie._parent, {})
        self.compareDict(self._trie._vals, {})
        self.compareDict(self._trie._out, {})
        self.compareDict(self._trie._keywords, {})
        self.compareDict(self._trie._keywordIds, {})
        self.compareSet(self._trie._accept, set())

        self.assertEqual(self._trie._currentNodeId, 0)
        self.assertEqual(self._trie._currentKeywordId, -1)
        self.assertEqual(len(self._trie._nodes), 1)

        m_clearCache.assert_called_with()
    
    # ================================================
    # ============ _compareKeywordIds ================

    def test_differentKeywordIds_properKeywordIdCompareResult(self):
        tests = [[None, None, 0],
                 [None, "app", 1],
                 ["pp", None, -1],
                 ["app", "apple", 1],
                 ["pls", "pp", -1],
                 ["apple", "appls", -1],
                 ["pp", "le", 1],
                 ["shappy", "shappy", 0]]
        
        for test in tests:
            keyword1 = test[0]
            keyword2 = test[1]

            id1 = keyword1 if (keyword1 is None) else self._trie._keywordIds[keyword1]
            id2 = keyword2 if (keyword2 is None) else self._trie._keywordIds[keyword2]

            expected = test[2]
            result = self._trie._compareKeywordIds(id1, id2)
            self.assertEqual(result, expected)

    # ================================================
    # =================== build ======================

    def test_differentKVPData_trieBuilt(self):
        tests = [[{}, 1],
                 [{"": 1}, 1],
                 [{"singleton": 1}, 10],
                 [{"abc": 1, "ab": 2, "abd": 3, "cfg": 4, "": 5}, 8],
                 [self._trieData, 23]]
        
        for test in tests:
            data = test[0]
            dataLen = len(data)

            # if the trie T is a tree, we should get that:
            #   |E(T)| = |V(T)| - 1
            expectedVertices = test[1]
            expectedEdges = expectedVertices - 1

            self._trie.build(data)
            resultEdges = reduce(lambda acc, children: acc + len(children), self._trie._children.values(), 0)

            self.assertEqual(len(self._trie._nodes), expectedVertices)
            self.assertLessEqual(len(self._trie._children), expectedVertices)
            self.assertEqual(len(self._trie._vals), dataLen)
            self.assertEqual(len(self._trie._keywords), dataLen)
            self.assertEqual(len(self._trie._keywordIds), dataLen)
            self.assertEqual(len(self._trie._parent), expectedEdges)
            self.assertEqual(resultEdges, expectedEdges)
            self.assertEqual(len(self._trie._out), dataLen)
            self.assertEqual(len(self._trie._accept), dataLen)

            for nodeId in self._trie._out:
                self.assertEqual(len(self._trie._out[nodeId]), 1)

    # ================================================
    # ================ _addNode ======================

    def test_addMultipleTrieNodes_trieNodesAdded(self):
        self._trie.clear()
        nodesToAdd = 5

        for i in range(nodesToAdd):
            resultNode = self._trie._addNode()

            self.assertEqual(resultNode.id, i)
            self.assertEqual(len(self._trie._nodes), i + 2)

    # ================================================
    # ================ _addKVP =======================

    def test_addMultipleTrieKVPs_trieKVPsAdded(self):
        self._trie.clear()

        data = [["boooo", 0],
                ["", []],
                ["boot", FRB.Trie({})],
                ["boot", "my new boot"],
                ["shoelace", None],
                ["high heels", {}],
                ["😆😄", set()]]
        
        keywordsAdded = set()
        dataLen = len(data)

        for i in range(dataLen):
            kvp = data[i]
            keyword = kvp[0]
            value = kvp[1]

            self._trie._addKVP(keyword, value)
            keywordsAdded.add(keyword)

            keywordsAddedLen = len(keywordsAdded)
            self.assertEqual(len(self._trie._keywords), keywordsAddedLen)
            self.assertEqual(len(self._trie._keywordIds), keywordsAddedLen)
            self.assertEqual(len(self._trie._vals), keywordsAddedLen)

            self.assertIn(keyword, self._trie._keywordIds)
            keywordId = self._trie._keywordIds[keyword]

            self.assertEqual(self._trie._keywords[keywordId], keyword)
            self.assertEqual(type(self._trie._vals[keywordId]), type(value))

    # ================================================
    # ================== add =========================

    @mock.patch("src.py.FixRaidenBoss2.Trie._addKeyword")
    def test_differentKVPs_referencedTrieAddKeyword(self, m_addKeyword):
        data = [["boooo", 0, 5, True],
                ["", [], 0, True],
                ["boot", FRB.Algo(), 1, True],
                ["boot", "my new boot", 0, False],
                ["shoelace", None, 8, True],
                ["high heels", {}, 10, True],
                ["😆😄", set(), 2, True],
                ["high", 9.7, 0, True]]
        
        dataLen = len(data)

        for i in range(dataLen):
            kvpData = data[i]
            keyword = kvpData[0]
            value = kvpData[1] 

            self._trie._addKeyword(keyword, value)
            m_addKeyword.assert_called_with(keyword, value)

    # ================================================
    # ============= _addKeyword ======================

    def test_differentKVPS_trieWithKVPsAdded(self):
        self._trie.clear()
        data = [["boooo", 0, 5, True],
                ["", [], 0, True],
                ["boot", FRB.Algo(), 1, True],
                ["boot", "my new boot", 0, False],
                ["shoelace", None, 8, True],
                ["high heels", {}, 10, True],
                ["😆😄", set(), 2, True],
                ["high", 9.7, 0, True]]
        
        dataLen = len(data)
        keywordsAdded = set()
        expectedVertices = 1

        for i in range(dataLen):
            kvpData = data[i]
            keyword = kvpData[0]
            value = kvpData[1]

            # if the trie T is a tree, we should get that:
            #   |E(T)| = |V(T)| - 1
            expectedVertices += kvpData[2]
            expectedEdges = expectedVertices - 1
            expectedNewKeyword = kvpData[3]
            keywordsAdded.add(keyword)

            _, resultNewKeyword = self._trie._addKeyword(keyword, value)
            resultEdges = reduce(lambda acc, children: acc + len(children), self._trie._children.values(), 0)

            self.assertEqual(resultNewKeyword, expectedNewKeyword)
            self.assertEqual(len(self._trie._nodes), expectedVertices)
            self.assertEqual(resultEdges, expectedEdges)

            keywordsAddedLen = len(keywordsAdded)
            self.assertEqual(len(self._trie._keywords), keywordsAddedLen)
            self.assertEqual(len(self._trie._keywordIds), keywordsAddedLen)
            self.assertEqual(len(self._trie._vals), keywordsAddedLen)
            self.assertEqual(len(self._trie._accept), keywordsAddedLen)

    # ================================================
    # =================== get ========================

    def test_keywordInTrie_valueRetrieved(self):
        tests = [["shappy"],
                 ["s"],
                 ["pls"],
                 ["apple"],
                 ["le"],
                 ["shappyer"]]
        
        for test in tests:
            keyword = test[0]
            expected = self._trieData[keyword]
            result = self._trie.get(keyword)
            self.assertEqual(result, expected)

    def test_keywordsNotInTrieRaiseError_errorRaised(self):
        tests = ["a", "bbbbb", "shappye", "let", "", "apples"]

        for test in tests:
            result = None
            try:
                self._trie.get(test)
            except Exception as e:
                result = e

            self.assertIsNot(result, None)
            self.assertIsInstance(result, KeyError)

    def test_keywordsNotInTrieWithDefault_defaultReturned(self):
        tests = [["a", None],
                 ["plo", ""],
                 ["shappye", 9],
                 ["", 45],
                 ["let", 9.7]]
        
        for test in tests:
            keyword = test[0]
            default = test[1]

            result = self._trie.get(keyword, errorOnNotFound = False, default = default)
            self.assertEqual(result, default)

    # ================================================