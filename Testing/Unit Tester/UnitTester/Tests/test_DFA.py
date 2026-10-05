import sys
import unittest.mock as mock
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys
from typing import Hashable, List, Tuple

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class DFATest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        
        cls.dfa = FRB.DFA()
    
    def buildDFA(self, states: List[Hashable], transitions: List[Tuple[Hashable, Hashable, Hashable]], clearDFA: bool = True):
        if (clearDFA):
            self.dfa.clear()

        for state in states:
            self.dfa.addState(state)

        for transition in transitions:
            self.dfa.addTransition(transition[0], transition[1], transition[2])

    def buildDefaultDFA(self):
        # Petersen graph with self states
        startStateId = "outer1"
        states = [startStateId, "outer2", "outer3", "outer4", "outer5", "inner1", "inner2", "inner3", "inner4", "inner5" ]
        transitions = [
            ("outer1", "left", "outer5"),
            ("outer1", "right", "outer2"),
            ("outer1", "front", "inner1"),
            ("outer2", "left", "outer1"),
            ("outer2", "right", "outer3"),
            ("outer2", "front", "inner2"),
            ("outer3", "left", "outer2"),
            ("outer3", "right", "outer4"),
            ("outer3", "front", "inner3"),
            ("outer4", "left", "outer3"),
            ("outer4", "right", "outer5"),
            ("outer4", "center", "inner4"),
            ("outer5", "left", "outer4"),
            ("outer5", "right", "outer1"),
            ("outer5", "front", "inner5"),
            ("inner1", "left", "inner3"),
            ("inner1", "right", "inner4"),
            ("inner1", "back", "outer1"),
            ("inner2", "left", "inner5"),
            ("inner2", "right", "inner4"),
            ("inner2", "back", "outer2"),
            ("inner3", "left", "inner1"),
            ("inner3", "right", "inner5"),
            ("inner3", "back", "outer3"),
            ("inner4", "left", "inner2"),
            ("inner4", "right", "inner1"),
            ("inner4", "back", "outer4"),
            ("inner5", "left", "inner2"),
            ("inner5", "right", "inner3"),
            ("inner5", "back", "outer5")
        ]

        for state in states:
            transitions.append((state, "stay", state))

        self.buildDFA(states, transitions)

        finishStateId = "finish"
        self.dfa.addState(finishStateId, isAccept = True)
        for state in states:
            self.dfa.addTransitions(state, ["end", "fini"], finishStateId)

        self.dfa.addTransitions(finishStateId, "continue", startStateId)
        
        self.dfa.addTransition(startStateId, lambda keyword: keyword.startswith("e"), finishStateId)
        self.dfa.addTransition(startStateId, lambda keyword: len(keyword) == 2, "inner2")

    def setUp(self):
        super().setUp()
        self.buildDefaultDFA()

    # ============ startId.setter ====================

    def test_differentStateIds_newStartIdSetIfExist(self):
        tests = [["outer2", None],
                 ["outer2", None],
                 [1, KeyError]]
        
        for test in tests:
            newStartId = test[0]
            expectedError = test[1]

            resultError = None
            try:
                self.dfa.startId = newStartId
            except Exception as e:
                resultError = e

            if (expectedError is not None):
                self.assertIsInstance(resultError, expectedError)
                continue

            self.assertIsNone(resultError)
            self.assertEqual(self.dfa.startId, newStartId)

    # ================================================
    # ========== currentStateId.setter ===============

    def test_differentStateIds_newCurrentIdSetIfExist(self):
        tests = [["outer5", None],
                 ["outer5", None],
                 [1, KeyError]]
        
        for test in tests:
            newCurrentStateId = test[0]
            expectedError = test[1]

            resultError = None
            try:
                self.dfa.currentStateId = newCurrentStateId
            except Exception as e:
                resultError = e

            if (expectedError is not None):
                self.assertIsInstance(resultError, expectedError)
                continue

            self.assertIsNone(resultError)
            self.assertEqual(self.dfa.currentStateId, newCurrentStateId)

    # ================================================
    # ================ clear =========================

    def test_dfaConstructed_dfaCleared(self):
        self.dfa.clear()

        self.assertEqual(self.dfa.stateLen(), 0)
        self.assertEqual(self.dfa.acceptLen(), 0)
        self.assertIsNone(self.dfa.startId)
        self.assertIsNone(self.dfa.currentStateId)

        self.dfa.clear()

        self.assertEqual(self.dfa.stateLen(), 0)
        self.assertEqual(self.dfa.acceptLen(), 0)
        self.assertIsNone(self.dfa.startId)
        self.assertIsNone(self.dfa.currentStateId)

    # ================================================
    # ================ addState ======================

    def test_differentStatesToAdd_statesAddedOrChangedType(self):
        tests = [[1, False, False, True],
                 [None, True, False, True],
                 [4.34, False, True, True],
                 ["", True, True, True],
                 ["inner1", True, False, False]]
        

        expectedStatesLen = self.dfa.stateLen()
        for test in tests:
            id = test[0]
            isAccept = test[1]
            isStart = test[2]
            expectedStateAdded = test[3]

            if (expectedStateAdded):
                expectedStatesLen += 1

            resultStateAdded = self.dfa.addState(id, isAccept = isAccept, isStart = isStart)

            assert(self.dfa.stateExists(id))
            self.assertEqual(resultStateAdded, expectedStateAdded)
            self.assertEqual(self.dfa.stateLen(), expectedStatesLen)
            self.assertEqual(self.dfa.isAccept(id), isAccept)
            self.assertEqual(id == self.dfa.startId, isStart)

    # ================================================
    # ============= addTransition ====================

    def test_differentTransitionsToAdd_transitionsAddedWithNewStatesAdded(self):
        tests = [["outer1", "skip1", "outer4", None, False],
                 ["inner1", "newPath", "island1", None, True],
                 ["unknown", "invalid", "theVoid", KeyError, False],
                 ["outer1", "skip1", "inner4", None, False],
                 ["outer1", "skip1", "inner4", None, False],
                 ["outer2", lambda keyword: keyword != "abc", "inner3", None, False]]
        
        expectedStatesLen = self.dfa.stateLen()
        for test in tests:
            srcStateId = test[0]
            destStateId = test[2]
            keyword = test[1]
            expectedError = test[3]
            expectedStateAdded = test[4]

            if (expectedStateAdded):
                expectedStatesLen += 1

            resultError = None
            try:
                self.dfa.addTransition(srcStateId, keyword, destStateId)
            except Exception as e:
                resultError = e

            if (expectedError is not None):
                self.assertIsInstance(resultError, expectedError)
                continue

            self.assertIsNone(resultError)
            self.assertEqual(self.dfa.stateLen(), expectedStatesLen)

    # ================================================
    # ============= addTransitions ===================

    ## TODO: explictely test the "addTransitions" function instead of just testing whether
    #   "addTransition" is called since that the DFA is all implemented in C++   
    ##
    # @mock.patch("src.py.FixRaidenBoss2.DFA.addTransition")
    # def test_differentGroupsofTransitionsToAdd_calledAddTransition(self, m_addTransition):
    #     tests = [["outer1", "skip1", "outer4", 1],
    #              ["outer1", lambda keyword: keyword == "a", "outer4", 1],
    #              ["outer1", [], "outer4", 0],
    #              ["outer1", ["hello", lambda keyword: keyword == "a", "boo"], "outer4", 3]]
        
    #     addTransitionCalled = 0
    #     for test in tests:
    #         addTransitionCalled += test[3]
    #         self.dfa.addTransitions(test[0], test[1], test[2])
    #         self.assertEqual(m_addTransition.call_count, addTransitionCalled)

    # ================================================
    # =============== transition =====================

    def test_differentTransitions_takeTransitionIfExist(self):
        tests = [["right", "outer2", False, True],
                 ["bogowalk", "outer2", False, False],
                 ["front", "inner2", False, True],
                 ["end", "finish", True, True],
                 ["flyaway", "finish", True, False],
                 ["continue", "outer1", False, True],
                 ["eb", "finish", True, True],
                 ["continue", "outer1", False, True],
                 ["ib", "inner2", False, True]]
        
        self.dfa.reset()
        for test in tests:
            keyword = test[0]
            expectedStateId = test[1]
            expectedIsAccept = test[2]
            expectedTransitionMade = test[3]

            resultStateId, resultIsAccept, resultTransitionMade = self.dfa.transition(keyword)

            self.assertEqual(resultTransitionMade, expectedTransitionMade)
            self.assertEqual(resultStateId, expectedStateId)
            self.assertEqual(resultIsAccept, expectedIsAccept)

    # ================================================
    # ========== getKeywordTransitions ================

    def test_differentStates_keywordTransitionsRetrieved(self):
        # func transitions (the 2 lambdas added onto "outer1" in buildDefaultDFA) are
        # deliberately excluded from every expected set below -- they aren't keywords.
        tests = [["outer1", True, {"left", "right", "front", "stay", "end", "fini"}],
                 ["outer3", True, {"left", "right", "front", "stay", "end", "fini"}],
                 ["inner2", True, {"left", "right", "back", "stay", "end", "fini"}],
                 ["finish", True, {"continue"}],
                 ["doesNotExist", False, set()]]

        for test in tests:
            stateId = test[0]
            expectedFound = test[1]
            expectedKeywords = test[2]

            resultKeywords, resultFound = self.dfa.getKeywordTransitions(stateId)

            self.assertEqual(resultFound, expectedFound)
            self.compareSet(set(resultKeywords), expectedKeywords)

    def test_noKeywordTransitions_emptyListReturned(self):
        self.dfa.clear()
        self.dfa.addState("lonely")

        resultKeywords, resultFound = self.dfa.getKeywordTransitions("lonely")

        self.assertTrue(resultFound)
        self.compareList(resultKeywords, [])

    # ================================================
    # =========== hasKeywordTransition ================

    def test_differentStatesAndKeywords_hasKeywordTransitionReturnsCorrectly(self):
        # func transitions (the 2 lambdas added onto "outer1" in buildDefaultDFA) are deliberately
        # excluded -- hasKeywordTransition only concerns itself with keyword transitions.
        tests = [["outer1", "left", True],
                 ["outer1", "right", True],
                 ["outer1", "stay", True],
                 ["outer1", "bogowalk", False],
                 ["outer3", "front", True],
                 ["inner2", "back", True],
                 ["inner2", "left", True],
                 ["finish", "continue", True],
                 ["finish", "left", False],
                 ["doesNotExist", "left", False]]

        for test in tests:
            srcId = test[0]
            keyword = test[1]
            expectedHasTransition = test[2]

            resultHasTransition = self.dfa.hasKeywordTransition(srcId, keyword)

            self.assertEqual(resultHasTransition, expectedHasTransition)

    def test_transitionOverwritten_hasKeywordTransitionStillReturnsTrue(self):
        self.assertTrue(self.dfa.hasKeywordTransition("outer1", "left"))

        # re-adding the same (srcId, keyword) with a different destination should still report
        # the transition as existing
        self.dfa.addTransition("outer1", "left", "inner3")

        self.assertTrue(self.dfa.hasKeywordTransition("outer1", "left"))

    def test_stateWithNoOutgoingTransitions_hasKeywordTransitionReturnsFalse(self):
        self.dfa.clear()
        self.dfa.addState("lonely")

        self.assertFalse(self.dfa.hasKeywordTransition("lonely", "anything"))

    # ================================================