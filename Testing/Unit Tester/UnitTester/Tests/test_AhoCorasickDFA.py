import sys
from functools import reduce
import unittest.mock as mock
from .baseTrieTest import BaseTrieTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class AhoCorasickDFATest(BaseTrieTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._trie = FRB.AhoCorasickDFA(cls._trieData)

    def setUp(self):
        super().setUp()
        self._trie = FRB.AhoCorasickDFA(self._trieData)


    # ============= __getitem__ ======================

    @mock.patch("src.FixRaidenBoss2.AhoCorasickDFA.getMaximal")
    def test_getItemFromAhoDFA_calledAhoDFAGet(self, m_get):
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

    @mock.patch("src.FixRaidenBoss2.AhoCorasickDFA.add")
    def test_setItemForAhoDFA_referencedAhoDFAAdd(self, m_add):
        data = [["boooo", 0],
                ["", []],
                ["boot", FRB.Algo()],
                ["boot", "my new boot"],
                ["shoelace", None],
                ["high heels", {}],
                ["😆😄", set()],
                ["high", 9.7]]
        
        dataLen = len(data)

        for i in range(dataLen):
            kvpData = data[i]
            keyword = kvpData[0]
            value = kvpData[1] 

            self._trie[keyword] = value
            m_add.assert_called_with(keyword, value)

    # ================================================
    # ============= __contains__ =====================

    @mock.patch("src.FixRaidenBoss2.AhoCorasickDFA.getMaximal")
    def test_getItemFromAhoDFA_calledAhoDFAGet(self, m_get):
        tests = [["dfd df shappy rr", True],
                 ["s", True],
                 ["pretty pls", True],
                 ["apple", True],
                 ["le", True],
                 ["shappyeri", True],
                 ["aaa", False],
                 ["", False],
                 ["the life of pi", False]]
        
        for test in tests:
            txt = test[0]
            expected = test[1]

            result = txt in self._trie
            self.assertEqual(result, expected)

    # ================================================
    # ================ clear =========================

    @mock.patch("src.FixRaidenBoss2.AhoCorasickDFA.clearCache")
    def test_AhoDFAWithData_AhoDFADataCleared(self, m_clearCache):
        self._trie.clear()

        self.compareDict(self._trie._children, {})
        self.compareDict(self._trie._parent, {})
        self.compareDict(self._trie._vals, {})
        self.compareDict(self._trie._out, {})
        self.compareDict(self._trie._keywords, {})
        self.compareDict(self._trie._keywordIds, {})
        self.compareDict(self._trie._fail, {})

        self.assertEqual(self._trie._currentNodeId, 0)
        self.assertEqual(self._trie._currentKeywordId, -1)
        self.assertEqual(len(self._trie._nodes), 1)
        m_clearCache.assert_called_with()
    
    # ================================================
    # ================= add ==========================

    def test_addNewKVP_reconstructAhoDFA(self):
        tests = [[{}, ("hello", 43110), 6, {0: 0, 1: 0, 2: 0, 3: 0, 4: 1}, {0: -1, 1: -1, 2: -1, 3: -1, 4: -1}],
                 [{"": 1}, ("boo", 800), 4, {-1: 1, 0: 1, 1: 1, 2: 2}, {0: -1, 1: -1, 2: -1}],
                 [{"singleton": 1}, ("multi", 888), 15, {0: 0, 1: 0, 2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0, 8: 1, 9: 0, 10: 0, 11: 0, 12: 0, 13: 1}, {0: -1, 1: -1, 2: -1, 3: -1, 4: -1, 5: -1, 6: -1, 7: -1, 8: -1, 9: -1, 10: -1, 11: -1, 12: -1, 13: -1}],
                 [{"abc": 1, "abd": 3, "cfg": 4, "": 5}, ("ab", 2), 8, {-1: 1, 0: 1, 1: 2, 2: 2, 3: 2, 4: 1, 5: 1, 6: 2}, {0: -1, 1: -1, 2: 4, 3: -1, 4: -1, 5: -1, 6: -1}],
                 [self._trieData, ("app", 299) , 23, 
                  {0: 1, 1: 0, 2: 0, 3: 0, 4: 2, 5: 1, 6: 0, 7: 1, 8: 0, 9: 0, 10: 2, 11: 0, 12: 2, 13: 3, 14: 1, 15: 0, 16: 1, 17: 0, 18: 1, 19: 0, 20: 2, 21: 1}, 
                  {0: -1, 1: -1, 2: 8, 3: 9, 4: 10, 5: -1, 6: -1, 7: -1, 8: -1, 9: 15, 10: 16, 11: 19, 12: 18, 13: 20, 14: -1, 15: -1, 16: 15, 17: -1, 18: -1, 19: 17, 20: 0, 21: -1}]]
        
        for test in tests:
            data = test[0]
            kvp = test[1]
            expectedOut = test[3]
            expectedFail = test[4]

            keyword = kvp[0]
            value = kvp[1]

            wordsAdded = set(data.keys())
            wordsAdded.add(keyword)
            dataLen = len(wordsAdded)

            # if the trie T is a tree, we should get that:
            #   |E(T)| = |V(T)| - 1
            expectedVertices = test[2]
            expectedEdges = expectedVertices - 1

            self._trie.build(data)
            self._trie.add(keyword, value)

            resultEdges = reduce(lambda acc, children: acc + len(children), self._trie._children.values(), 0)

            self.assertEqual(len(self._trie._nodes), expectedVertices)
            self.assertLessEqual(len(self._trie._children), expectedVertices)
            self.assertEqual(len(self._trie._vals), dataLen)
            self.assertEqual(len(self._trie._keywords), dataLen)
            self.assertEqual(len(self._trie._keywordIds), dataLen)
            self.assertEqual(len(self._trie._parent), expectedEdges)
            self.assertEqual(resultEdges, expectedEdges)

            resultOut = {}
            for nodeId in self._trie._out:
                resultOut[nodeId] = len(self._trie._out[nodeId])

            self.compareDict(resultOut, expectedOut)
            self.compareDict(self._trie._fail, expectedFail)

    # ================================================
    # =================== build ======================

    def test_differentKVPData_AhoDFABuilt(self):
        tests = [[{}, 1, {}, {}],
                 [{"": 1}, 1, {-1: 1}, {}],
                 [{"singleton": 1}, 10, {0: 0, 1: 0, 2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0, 8: 1}, {0: -1, 1: -1, 2: -1, 3: -1, 4: -1, 5: -1, 6: -1, 7: -1, 8: -1}],
                 [{"abc": 1, "ab": 2, "abd": 3, "cfg": 4, "": 5}, 8, {-1: 1, 0: 1, 1: 2, 2: 2, 3: 2, 4: 1, 5: 1, 6: 2}, {0: -1, 1: -1, 2: 4, 3: -1, 4: -1, 5: -1, 6: -1}],
                 [self._trieData, 23, 
                  {0: 1, 1: 0, 2: 0, 3: 0, 4: 2, 5: 1, 6: 0, 7: 1, 8: 0, 9: 0, 10: 2, 11: 0, 12: 2, 13: 3, 14: 1, 15: 0, 16: 1, 17: 0, 18: 1, 19: 0, 20: 2, 21: 1}, 
                  {0: -1, 1: -1, 2: 8, 3: 9, 4: 10, 5: -1, 6: -1, 7: -1, 8: -1, 9: 15, 10: 16, 11: 19, 12: 18, 13: 20, 14: -1, 15: -1, 16: 15, 17: -1, 18: -1, 19: 17, 20: 0, 21: -1}]]
        
        for test in tests:
            data = test[0]
            dataLen = len(data)
            expectedOut = test[2]
            expectedFail = test[3]

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

            resultOut = {}
            for nodeId in self._trie._out:
                resultOut[nodeId] = len(self._trie._out[nodeId])

            self.compareDict(resultOut, expectedOut)
            self.compareDict(self._trie._fail, expectedFail)

    # ================================================
    # ================= findAll ======================

    def test_differentSearchStr_keywordsFound(self):
        tests = [[{}, "", {}],
                 [{}, "abcde", {}],
                 [{"": 1}, "abcde", {"": [(0,0), (1,1), (2,2), (3,3), (4,4), (5,5)]}],
                 [{"singleton": 1}, "this is not a singleton structure", {"singleton": [(14, 23)]}],
                 [{"abc": 1, "ab": 2, "abd": 3, "cfg": 4, "": 5}, "abcabdtyabcfgabcfg", {"abc": [(0, 3), (8, 11), (13, 16)], 
                                                                                         "ab": [(0, 2), (3, 5), (8, 10), (13, 15)],
                                                                                         "abd": [(3, 6)],
                                                                                         "cfg": [(10, 13), (15, 18)],
                                                                                         "": [(0,0), (1,1), (2,2), (3,3), (4,4), (5,5), (6,6), (7,7), (8,8), (9,9),
                                                                                              (10,10), (11,11), (12,12), (13,13), (14,14), (15,15), (16,16), (17,17), (18,18)]}],
                [self._trieData, "shappay", {"app": [(2, 5)],
                                             "pp": [(3, 5)],
                                             "s": [(0, 1)]}],
                [self._trieData, "shappyer", {"shappyer": [(0, 8)],
                                              "shappy": [(0, 6)],
                                              "app": [(2, 5)],
                                              "pp": [(3, 5)],
                                              "s": [(0, 1)]}],
                [self._trieData, "eat an applse a day will not keep the doctor away", {"app": [(7, 10)],
                                                                                       "appls": [(7, 12)],
                                                                                       "pp": [(8, 10)],
                                                                                       "pls": [(9, 12)],
                                                                                       "s": [(11, 12)]}],
                [self._trieData, "pear and banana", {}]]

        for test in tests:
            data = test[0]
            txt = test[1]
            expected = test[2]

            self._trie.build(data)
            result = self._trie.findAll(txt)

            self.compareDict(result, expected, 
                             lambda resKeywordInds, expectedKeywordInds: self.compareList(resKeywordInds, expectedKeywordInds, 
                                                                                          lambda resInds, expectedInds: self.compareList(resInds, expectedInds)))

    # ================================================
    # ================= find =========================

    def test_differentSearchStr_firstKeywordFound(self):
        tests = [[{}, "", None, -1],
                 [{}, "abcde", None, -1],
                 [{"": 1}, "abcde", "", 0],
                 [{"singleton": 1}, "this is not a singleton structure", "singleton", 14],
                 [{"abc": 1, "ab": 2, "abd": 3, "cfg": 4, "": 5}, "abcabdtyabcfgabcfg", "", 0],
                [self._trieData, "shappay", "s", 0],
                [self._trieData, "shappyer", "s", 0],
                [self._trieData, "eat an applse a day will not keep the doctor away", "app", 7],
                [self._trieData, "pear and banana", None, -1]]

        for test in tests:
            data = test[0]
            txt = test[1]
            expectedKeyword = test[2]
            expectedInd = test[3]

            self._trie.build(data)
            resultKeyword, resultInd = self._trie.find(txt)

            self.assertEqual(resultKeyword, expectedKeyword)
            self.assertEqual(resultInd, expectedInd)

    # ================================================
    # ============== findMaximal =====================

    def test_differentSearchStr_firstMaximalKeywordFound(self):
        tests = [[{}, "", None, -1],
                 [{}, "abcde", None, -1],
                 [{"": 1}, "abcde", "", 0],
                 [{"singleton": 1}, "this is not a singleton structure", "singleton", 14],
                 [{"abc": 1, "ab": 2, "abd": 3, "cfg": 4, "": 5}, "abcabdtyabcfgabcfg", "abc", 0],
                [self._trieData, "shappay", "s", 0],
                [self._trieData, "shappyer", "shappyer", 0],
                [self._trieData, "eat an applse a day will not keep the doctor away", "appls", 7],
                [self._trieData, "pear and banana", None, -1]]

        for test in tests:
            data = test[0]
            txt = test[1]
            expectedKeyword = test[2]
            expectedInd = test[3]

            self._trie.build(data)
            resultKeyword, resultInd = self._trie.findMaximal(txt)

            self.assertEqual(resultKeyword, expectedKeyword)
            self.assertEqual(resultInd, expectedInd)

    def test_differentSearchStr_firstFewMaximalKeywordsFound(self):
        tests = [[{}, "", 5, [], []],
                [{}, "abcde", 6, [], []],
                [{"": 1}, "abcde", 30, ["", "", "", "", "", ""], [0, 1, 2, 3, 4, 5]],
                [{"singleton": 1}, "this is not a singleton structure", 3, ["singleton"], [14]],
                [{"abc": 1, "ab": 2, "abd": 3, "cfg": 4, "": 5}, "abcabdtyabcfgabcfg", 100, ["abc", "abd", "", "", "abc", "", "", "abc", "", "", ""], [0, 3, 6, 7, 8, 11, 12, 13, 16, 17, 18]],
            [self._trieData, "shappay", 2, ["s", "app"], [0, 2]],
            [self._trieData, "shappyer", 2, ["shappyer"], [0]],
            [self._trieData, "eat an applse a day will not keep the doctor away", 6, ["appls"], [7]],
            [self._trieData, "pear and banana", 500, [], []]]

        for test in tests:
            data = test[0]
            txt = test[1]
            count = test[2]
            expectedKeywords = test[3]
            expectedInds = test[4]

            self._trie.build(data)
            resultKeywords, resultInds = self._trie.findMaximal(txt, count = count)

            self.compareList(resultKeywords, expectedKeywords)
            self.compareList(resultInds, expectedInds)

    # ================================================
    # ================== get =========================

    def test_txtHasKeyword_firstKeywordValueRetrieved(self):
        tests = [[{"": "noVal"}, "abcde", ("", "noVal")],
                 [{"singleton": "monopoly"}, "this is not a singleton structure", ("singleton", "monopoly")],
                 [{"abc": 1, "ab": 2, "abd": 3, "cfg": 4, "": 5}, "abcabdtyabcfgabcfg", ("", 5)],
                [self._trieData, "shappay", ("s", "s value")],
                [self._trieData, "shappyer", ("s", "s value")],
                [self._trieData, "eat an applse a day will not keep the doctor away", ("app", "app value")]]

        for test in tests:
            data = test[0]
            txt = test[1]
            expected = test[2]

            self._trie.build(data)
            result = self._trie.get(txt)

            self.compareList(result, expected)

    def test_txtHasNoKeywords_errorRaisedWithoutFirstKeywordVal(self):
        tests = [[{}, ""],
                 [{}, "abcde"],
                [self._trieData, "pear and banana"]]

        for test in tests:
            data = test[0]
            txt = test[1]

            self._trie.build(data)

            result = None
            try:
                self._trie.get(txt)
            except Exception as e:
                result = e

            self.assertIsInstance(result, KeyError)

    def test_txtHasNoKeywords_defaultValWithoutFirstKeywordVal(self):
        tests = [[{}, "", None],
                 [{}, "abcde", 0.8888888],
                [self._trieData, "pear and banana", 909090909090909]]

        for test in tests:
            data = test[0]
            txt = test[1]
            expected = test[2]

            self._trie.build(data)
            keyword, result = self._trie.get(txt, errorOnNotFound = False, default = expected)

            self.assertIsNone(keyword)
            self.assertEqual(result, expected)

    # ================================================
    # =============== getMaximal =====================

    def test_txtHasKeyword_firstMaximalKeywordValueRetrieved(self):
        tests = [[{"": "noVal"}, "abcde", ("", "noVal")],
                 [{"singleton": "monopoly"}, "this is not a singleton structure", ("singleton", "monopoly")],
                 [{"abc": "alphabetinc", "ab": 2, "abd": 3, "cfg": 4, "": 5}, "abcabdtyabcfgabcfg", ("abc", "alphabetinc")],
                [self._trieData, "shappay", ("s", "s value")],
                [self._trieData, "shappyer", ("shappyer", "shappyer value")],
                [self._trieData, "eat an applse a day will not keep the doctor away", ("appls", "appls value")]]

        for test in tests:
            data = test[0]
            txt = test[1]
            expected = test[2]

            self._trie.build(data)
            result = self._trie.getMaximal(txt)

            self.compareList(result, expected)

    def test_txtHasKeyword_firstFewMaximalKeywordValueRetrieved(self):
        tests = [[{"": "noVal"}, "abcde", 5, (["", "", "", "", ""], ["noVal", "noVal", "noVal", "noVal", "noVal"])],
                 [{"singleton": "monopoly"}, "this is not a singleton structure", 6, (["singleton"], ["monopoly"])],
                 [{"abc": "alphabetinc", "ab": 2, "abd": 3, "cfg": 4, "": 5}, "abcabdtyabcfgabcfg", 30, (["abc", "abd", "", "", "abc", "", "", "abc", "", "", ""], ["alphabetinc", 3, 5, 5, "alphabetinc", 5, 5, "alphabetinc", 5, 5, 5])],
                [self._trieData, "shappay", 3, (["s", "app"], ["s value", "app value"])],
                [self._trieData, "shappyer", 2, (["shappyer"], ["shappyer value"])],
                [self._trieData, "eat an applse a day will not keep the doctor away", 100, (["appls"], ["appls value"])]]

        for test in tests:
            data = test[0]
            txt = test[1]
            count = test[2]

            expected = test[3]
            expectedKeys = expected[0]
            expectedVals = expected[1]

            self._trie.build(data)
            resultKeys, resultVals = self._trie.getMaximal(txt, count = count)

            self.compareList(resultKeys, expectedKeys)
            self.compareList(resultVals, expectedVals)

    def test_txtHasNoKeywords_errorRaisedWithoutFirstMaximalKeywordVal(self):
        tests = [[{}, ""],
                 [{}, "abcde"],
                [self._trieData, "pear and banana"]]

        for test in tests:
            data = test[0]
            txt = test[1]

            self._trie.build(data)

            result = None
            try:
                self._trie.getMaximal(txt)
            except Exception as e:
                result = e

            self.assertIsInstance(result, KeyError)

    def test_txtHasNoKeywords_defaultValWithoutFirstMaximalKeywordVal(self):
        tests = [[{}, "", None],
                 [{}, "abcde", 0.8888888],
                [self._trieData, "pear and banana", 909090909090909]]

        for test in tests:
            data = test[0]
            txt = test[1]
            expected = test[2]

            self._trie.build(data)
            keyword, result = self._trie.getMaximal(txt, errorOnNotFound = False, default = expected)

            self.assertIsNone(keyword)
            self.assertEqual(result, expected)

    def test_txtHasNoKeywords_emptyMaximalKeywordLists(self):
        tests = [[{}, "", 3, None],
                 [{}, "abcde", 200, 0.8888888],
                [self._trieData, "pear and banana", 40, 909090909090909]]

        for test in tests:
            data = test[0]
            txt = test[1]
            count = test[2]
            default = test[3]
            expectedKeys = []
            expectedVals = []

            self._trie.build(data)
            resultKeys, resultVals = self._trie.getMaximal(txt, errorOnNotFound = False, default = default, count = count)

            self.compareList(resultKeys, expectedKeys)
            self.compareList(resultVals, expectedVals)

    # ================================================
    # ================ getAll ========================

    def test_txtHasKeywords_allKeywordValsFound(self):
        tests = [[{"": "noVal"}, "abcde", {"": "noVal"}],
                 [{"singleton": "monopoly"}, "this is not a singleton structure", {"singleton": "monopoly"}],
                 [{"abc": "alphabetinc", "ab": 2, "abd": 3, "cfg": 4, "": 5}, "abcabdtyabcfgabcfg", {"abc": "alphabetinc", "ab": 2, "abd": 3, "cfg": 4, "": 5}],
                [self._trieData, "shappay", {"s": "s value", "app": "app value", "pp": "pp value"}],
                [self._trieData, "shappyer", {"shappyer": "shappyer value", "shappy": "shappy value", "s": "s value", "app": "app value", "pp": "pp value"}],
                [self._trieData, "eat an applse a day will not keep the doctor away", {"appls": "appls value",  "app": "app value", "pp": "pp value", "pls": "pls value", "s": "s value"}]]

        for test in tests:
            data = test[0]
            txt = test[1]
            expected = test[2]

            self._trie.build(data)
            result = self._trie.getAll(txt)

            self.compareDict(result, expected)

    # ================================================


    def test_modIni_compareAhoCorasickAndManyRegex(self):
        import re
        from timeit import default_timer as timer

        search = """
            ; HuTaoCherry

            ; Constants -------------------------

            ; Overrides -------------------------

            [TextureOverrideHuTaoCherryPosition]
            hash = a78db232
            vb0 = ResourceHuTaoCherryPosition

            [TextureOverrideHuTaoCherryBlend]
            hash = 6e718139
            vb1 = ResourceHuTaoCherryBlend
            handling = skip
            draw = 69708,0 

            [TextureOverrideHuTaoCherryTexcoord]
            hash = 4b14b10e
            vb1 = ResourceHuTaoCherryTexcoord

            [TextureOverrideHuTaoCherryVertexLimitRaise]
            hash = 6715905e
            ; override_vertex_count = 28708
            ; override_byte_stride = 100

            [TextureOverrideHuTaoCherryIB]
            hash = 92fce51e
            handling = skip
            drawindexed = auto

            [TextureOverrideHuTaoCherryHead]
            hash = 92fce51e
            match_first_index = 0
            ib = ResourceHuTaoCherryHeadIB
            ps-t0 = ResourceHuTaoCherryHeadNormalMap
            ps-t1 = ResourceHuTaoCherryHeadDiffuse
            ps-t2 = ResourceHuTaoCherryHeadLightMap
            run = CommandList\global\ORFix\ORFix
            [TextureOverrideHuTaoCherryBody]
            hash = 92fce51e
            match_first_index = 43968
            ib = ResourceHuTaoCherryBodyIB
            ps-t0 = ResourceHuTaoCherryBodyDiffuse
            ps-t1 = ResourceHuTaoCherryBodyLightMap

            [TextureOverrideHuTaoCherryDress]
            hash = 92fce51e
            match_first_index = 77301
            ib = ResourceHuTaoCherryDressIB
            ps-t0 = ResourceHuTaoCherryDressNormalMap
            ps-t1 = ResourceHuTaoCherryDressDiffuse
            ps-t2 = ResourceHuTaoCherryDressLightMap
            run = CommandList\global\ORFix\ORFix
            [TextureOverrideHuTaoCherryExtra]
            hash = 92fce51e
            match_first_index = 86808
            ib = ResourceHuTaoCherryExtraIB
            ps-t0 = ResourceHuTaoCherryExtraDiffuse

            ; CommandList -----------------------

            ; Resources -------------------------

            [ResourceHuTaoCherryPosition]
            type = Buffer
            stride = 40
            filename = HuTaoCherryPosition.buf

            [ResourceHuTaoCherryBlend]
            type = Buffer
            stride = 32
            filename = HuTaoCherryBlend.buf

            [ResourceHuTaoCherryTexcoord]
            type = Buffer
            stride = 28
            filename = HuTaoCherryTexcoord.buf

            [ResourceHuTaoCherryHeadIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = HuTaoCherryHead.ib

            [ResourceHuTaoCherryBodyIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = HuTaoCherryBody.ib

            [ResourceHuTaoCherryDressIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = HuTaoCherryDress.ib

            [ResourceHuTaoCherryExtraIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = HuTaoCherryExtra.ib

            [ResourceHuTaoCherryHeadNormalMap]
            filename = HuTaoCherryHeadNormalMap.dds

            [ResourceHuTaoCherryHeadDiffuse]
            filename = HuTaoCherryHeadDiffuse.dds

            [ResourceHuTaoCherryHeadLightMap]
            filename = HuTaoCherryHeadLightMap.dds

            [ResourceHuTaoCherryBodyDiffuse]
            filename = HuTaoCherryBodyDiffuse.dds

            [ResourceHuTaoCherryBodyLightMap]
            filename = HuTaoCherryBodyLightMap.dds

            [ResourceHuTaoCherryDressNormalMap]
            filename = HuTaoCherryDressNormalMap.dds

            [ResourceHuTaoCherryDressDiffuse]
            filename = HuTaoCherryDressDiffuse.dds

            [ResourceHuTaoCherryDressLightMap]
            filename = HuTaoCherryDressLightMap.dds

            [ResourceHuTaoCherryExtraDiffuse]
            filename = HuTaoCherryExtraDiffuse.dds


            [ResourceHuTaoCherryBodyDiffuseRemap]
            filename = HuTaoCherryBodyDiffuseRemap.dds


            ; --------------- CherryHuTao Remap ---------------
            ; CherryHuTao remapped by Albert Gold#2696 and NK#1321. If you used it to remap your CherryHuTao mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** HuTao *****
            [TextureOverrideHuTaoCherryHuTaoRemapBlend]
            hash = 153dba3f
            vb1 = ResourceHuTaoCherryHuTaoRemapBlend
            handling = skip
            draw = 69708,0


            [TextureOverrideHuTaoCherryPositionHuTaoRemapFix]
            hash = dd16576c
            vb0 = ResourceHuTaoCherryPosition

            [TextureOverrideHuTaoCherryTexcoordHuTaoRemapFix]
            hash = 51afdfcf
            vb1 = ResourceHuTaoCherryTexcoord

            [TextureOverrideHuTaoCherryVertexLimitRaiseHuTaoRemapFix]
            hash = e9d17db6

            [TextureOverrideHuTaoCherryIBHuTaoRemapFix]
            hash = 3de1efe2
            handling = skip
            drawindexed = auto

            [TextureOverrideHuTaoCherryHeadHuTaoRemapFix]
            hash = 3de1efe2
            match_first_index = 0
            ib = ResourceHuTaoCherryHeadIB
            ps-t0 = ResourceHuTaoCherryHeadDiffuse
            ps-t1 = ResourceHuTaoCherryHeadLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideHuTaoCherryBodyHuTaoRemapFix]
            hash = 3de1efe2
            match_first_index = 16509
            ib = ResourceHuTaoCherryBodyIB
            ps-t0 = ResourceCherryHuTaoBodyTransparentBodyDiffuseHuTaoRemapTex0
            ps-t1 = ResourceHuTaoCherryBodyLightMap


            [ResourceHuTaoCherryHuTaoRemapBlend]
            type = Buffer
            stride = 32
            filename = HuTaoCherryHuTaoRemapBlend.buf

            [ResourceCherryHuTaoBodyTransparentBodyDiffuseHuTaoRemapTex0]
            filename = HuTaoCherryBodyDiffuseHuTaoRemapTex0.dds

            ; *****************

            ; -------------------------------------------------
        """

        search = FRB.TextTools.getTextLines(search)

        
        regexes = [
            re.compile("TextureOverrideJean"),
            re.compile("TextureOverrideJeanSea"),
            re.compile("TextureOverrideJeanCN"),
            re.compile("TextureOverrideAmber"),
            re.compile("TextureOverrideAmberCN"),
            re.compile("TextureOverrideRosaria"),
            re.compile("TextureOverrideRosariaCN"),
            re.compile("TextureOverrideMona"),
            re.compile("TextureOverrideMonaCN"),
            re.compile("TextureOverrideRaiden"),
            re.compile("TextureOverrideArlecchino"),
            re.compile("TextureOverrideGanyu"),
            re.compile("TextureOverrideGanyuOrchid"),
            re.compile("TextureOverrideShenhe"),
            re.compile("TextureOverrideShenheFrostFlower"),
            re.compile("TextureOverrideHuTao"),
            re.compile("TextureOverrideHuTaoCherry"),
            re.compile("TextureOverrideCherryHuTao"),
            re.compile("TextureOverrideKirara"),
            re.compile("TextureOverrideKiraraBoots"),
            re.compile("TextureOverrideDiluc"),
            re.compile("TextureOverrideDilucFlamme"),
            re.compile("TextureOverrideFischl"),
            re.compile("TextureOverrideFischlHighness"),
            re.compile("TextureOverrideNingguang"),
            re.compile("TextureOverrideNingguangOrchid"),
            re.compile("TextureOverrideKeqing"),
            re.compile("TextureOverrideKeqingOpulent"),
            re.compile("TextureOverrideKlee"),
            re.compile("TextureOverrideKleeBlossomingStarlight"),
            re.compile("TextureOverrideNilou"),
            re.compile("TextureOverrideNilouBreeze"),
            re.compile("TextureOverrideXingqiu"),
            re.compile("TextureOverrideXingqiuBamboo"),
        ]


        dfa = FRB.AhoCorasickDFA({
            "TextureOverrideJean": 0,
            "TextureOverrideJeanSea": 0,
            "TextureOverrideJeanCN": 0,
            "TextureOverrideAmber": 0,
            "TextureOverrideAmberCN": 0,
            "TextureOverrideRosaria": 0,
            "TextureOverrideRosariaCN": 0,
            "TextureOverrideMona": 0,
            "TextureOverrideMonaCN": 0,
            "TextureOverrideRaiden": 0,
            "TextureOverrideArlecchino": 0,
            "TextureOverrideGanyu": 0,
            "TextureOverrideGanyuTwilight": 0,
            "TextureOverrideShenhe": 0,
            "TextureOverrideShenheFrostFlower": 0,
            "TextureOverrideHuTao": 0,
            "TextureOverrideHuTaoCherry": 0,
            "TextureOverrideCherryHuTao": 0,
            "TextureOverrideKirara": 0,
            "TextureOverrideKiraraBoots": 0,
            "TextureOverrideDiluc": 0,
            "TextureOverrideDilucFlamme": 0,
            "TextureOverrideFischl": 0,
            "TextureOverrideFischlHighness": 0,
            "TextureOverrideNingguang": 0,
            "TextureOverrideNingguangOrchid": 0,
            "TextureOverrideKeqing": 0,
            "TextureOverrideKeqingOpulent": 0,
            "TextureOverrideKlee": 0,
            "TextureOverrideKleeBlossomingStarlight": 0,
            "TextureOverrideNilou": 0,
            "TextureOverrideNilouBreeze": 0,
            "TextureOverrideXingqiu": 0,
            "TextureOverrideXingqiuBamboo": 0
        })

        # regexes = [
        #     re.compile("^((?!Remap).)*Blend$"),
        #     re.compile("^Position((?!RemapFix).)*$")
        # ]

        # dfa = FRB.AhoCorasickDFA({
        #     "Blend": 0,
        #     "Position": 0,
        #     "RemapBlend": -1,
        #     "RemapFix": -1
        # })

        def tempRegex(line):
            for reg in regexes:
                if (re.search(reg, line)):
                    return True
                
            return False


        def tempFunc(line):
            keyword, _ = dfa.findMaximal(line)
            return keyword is not None
        
        start = timer()
        for line in search:
            regResult = tempRegex(line)
            if (regResult):
                break
        end = timer()
        regTime = end - start

        start = timer()
        for line in search:
            dfaResult = tempFunc(line)
            if (dfaResult):
                break
        end = timer()
        dfaTime = end - start

        # print(f"REGEX RESULT: {bool(regResult)}")
        # print(f"DFA RESULT: {bool(dfaResult)}")
        

        # print(f"REGEX TIME: {regTime}")
        # print(f"DFA TIME: {dfaTime}")

        # print(f"\nDFA IS WINNER: {dfaTime <= regTime}")
        # print(f"EFFICIENCY: {regTime / dfaTime}")