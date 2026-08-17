import sys
import copy
from sympy import Symbol, And, Eq, Not, Ne, simplify
from sympy.logic.boolalg import Boolean
from typing import Dict, Any, Union, List, Tuple, Optional, Union

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB

Part = Union[str, Dict[str, Any]]


class IniSectionGraphTest(BaseUnitTest):

    def compareIfContentPartColourVals(self, resultVal: Union[str, List[Tuple[int, str]]], expectedVal: Union[str, List[Tuple[int, str]]]):
        self.assertIs(type(resultVal), type(expectedVal))

        if (isinstance(expectedVal, list)):
            self.compareList(resultVal, expectedVal, lambda res, exp: self.compareList(res, exp))
        else:
            self.assertEqual(resultVal, expectedVal)

    def compareIfContentPartColouring(self, result: FRB.IfContentPartColouring, expected: FRB.IfContentPartColouring):
        self.compareDict(result, expected, lambda resultVal, expectedVal: self.compareIfContentPartColourVals(resultVal, expectedVal))

    # ====================== build ===========================

    def test_differentIniGraphs_iniGraphsBuilt(self):
        tests = [
                 [{}, [], {}, []],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, [], {}, []],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], {}, ["root"]],
                 [{"loop": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0),
                                           FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                           FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "loop")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)])},
                  ["loop"], {"loop": ["loop"]}, ["loop"]],

                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                    [".", ".a"], {".": [".a", ".b"], ".a": [".aa", ".ab"]}, ["."]],
                    
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, ".")], "cacheType": []}, 0)])},
                    [".a", ".b"], {".": [".a", ".b"], ".a": [".aa", ".ab"], ".ab": ["."]}, [".a"]],
                    
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, ".ba")]}, 0)]),
                    ".ba": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                    [".aa", ".ab", ".ba", ".b", ".a"], {".a": [".aa", ".ab"], ".b": [".ba"]}, [".a", ".b"]],
                    
                  [{"a": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "b")]}, 0)]),
                    "b": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, "a")]}, 0)])},
                    ["a", "b"], {"a": ["b"], "b": ["a"]}, ["a"]]
                    ]

        for test in tests:
            sections = test[0]
            targetSectionNames = test[1]

            graph = FRB.IniSectionGraph(sections, targetSectionNames, build = False)
            graph.build()

            expectedNeighbours = test[2]
            expectedRoots = test[3]

            self.compareDict(graph.neighbours, expectedNeighbours, lambda resVal, expVal: self.compareList(resVal, expVal))
            self.compareList(graph.roots, expectedRoots)

    # ========================================================
    # =================== getChildren ========================

    def test_differentIniGraphs_dfsChildrenRetrieved(self):
        tests = [
            [{}, [], [], True, {}],
            [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, [], ["root", "bang"], True, {"root": set(), "bang": set()}],
            [{"loop": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0),
                                           FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                           FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "loop")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)])},
              ["loop"], ["loop"], True, {"loop": {"loop"}}],
            [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                    FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                    FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                    FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                    FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, ".aaa")]}, 0)]),
                ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aaa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                [".", ".a"], ["."], True, {".": {".a", ".b", ".aa", ".ab", ".aaa"}, ".a": {".aa", ".ab", ".aaa"}, ".b": set(), ".aa": {".aaa"}, ".ab": set(), ".aaa": set()}],
            
            [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                    FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                    FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                    FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                    FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, ".aaa")]}, 0)]),
                ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aaa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                [".", ".a"], ["."], True, {".": {".a", ".b", ".aa", ".ab", ".aaa"}, ".a": {".aa", ".ab", ".aaa"}, ".b": set(), ".aa": {".aaa"}, ".ab": set(), ".aaa": set()}],

            [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                    FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                    FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                    FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                    FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, ".aaa")]}, 0)]),
                ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aaa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                [".", ".a"], ["."], False, {".": {".a", ".b", ".aa", ".ab", ".aaa"}}],

            [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                    FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                    FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                    FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                    FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, ".aaa")]}, 0)]),
                ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aaa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                [".", ".a"], [".", ".aa"], False, {".": {".a", ".b", ".aa", ".ab", ".aaa"}, ".aa": {".aaa"}}],

            [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                    FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                    FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                    FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                    FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, ".bb")]}, 0)]),
                ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, ".aaa")]}, 0)]),
                ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aaa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, "connect")]}, 0)]),
                ".bb": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, "connect")]}, 0)]),
                "connect": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                [".", ".a"], [".aa", ".b"], False, {".aa": {".aaa", "connect"}, ".b": {".bb", "connect"}}],

            [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                    FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                    FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                    FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                    FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, ".bb")]}, 0)]),
                ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, ".aaa")]}, 0)]),
                ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                ".aaa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, "connect")]}, 0)]),
                ".bb": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [], "run": [(1, "connect")]}, 0)]),
                "connect": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": [],  "run": [(1, ".")]}, 0)])},
                [".", ".a"], [".aa", ".b"], False, {".aa": {".aaa", "connect", ".", ".a", ".b", ".ab", ".bb", ".aa"}, ".b": {".bb", "connect", ".", ".a", ".ab", ".aa", ".aaa", ".b"}}],
        ]

        for test in tests:
            sections = test[0]
            targetSectionNames = test[1]
            querySectionNames = test[2]
            getNeighbourChildren = test[3]

            graph = FRB.IniSectionGraph(sections, targetSectionNames)

            expectedChildren = test[4]

            resultChildren = graph.getChildren(querySectionNames, getNeighbourChildren = getNeighbourChildren)
            self.compareDict(resultChildren, expectedChildren, lambda resCurrentChildren, expCurrentChildren: self.compareSet(resCurrentChildren, expCurrentChildren))

    # ========================================================
    # ============ processIfContentByQuery ===================

    def test_differentIniGraphs1State_contentAndQueryProcessed(self):
        vars = {"x": Symbol("$x$"),
                "i": Symbol("$i$")}
        result = []
        process = lambda data: result.append((data.part, data.query, data.sectionName)) 

        tests = [
                 [{}, [], []],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], [(0, True, "root")]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                           FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, vars = vars),
                                            FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                                           FRB.IfPredPart("else", FRB.IfPredPartType.Else, vars = vars),
                                            FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, vars = vars)])}, ["root"],

                                           [(0, True, "root"),
                                            (2, Eq(vars["i"], 0), "root"),
                                            (4, And(Not(Eq(vars["i"], 0)), True), "root")]],

                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                           FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x + 7 == 5", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfPredPart("if $x - 7 == 2", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["root"],
                                           
                                           [(0, True, "root"),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "root"),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "root"),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "root"),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "root"),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "root"),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "root"),
                                            (20, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] + 7, 5)), "root"),
                                            (23, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] - 7, 2)), "root")]],
                                            
                 [{"loop": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                           FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                                                FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 2),
                                                FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["loop"],
                                           
                                           [(0, True, "loop"),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "loop"),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "loop"),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "loop"),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "loop"),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "loop"),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "loop")]],
                                            
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, ".")], "cacheType": []}, 0)])},
                    [".a", ".b"],
                    
                    [(1, Eq(vars["x"] / 3, 2), ".a"),
                     (0, Eq(vars["x"] / 3, 2), ".aa"),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a"),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab"),
                     (1, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Eq(vars["x"] + 3, 5)), "."),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), "."),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".b")]],
                     
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                    [".b", ".a"],
                    
                    [(0, True, ".b"),
                     (1, Eq(vars["x"] / 3, 2), ".a"),
                     (0, Eq(vars["x"] / 3, 2), ".aa"),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a"),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab")]]
                     ]
        
        for test in tests:
            sections = test[0]
            targetSectionNames = test[1]

            graph = FRB.IniSectionGraph(sections, targetSectionNames)

            expected = test[2]
            expected = list(map(lambda processLine: (sections[processLine[2]].parts[processLine[0]], processLine[1], processLine[2]), expected))

            result.clear()
            graph.processIfContentByQuery(process, simplify = True)

            resultLen = len(result)
            self.assertEqual(resultLen, len(expected))

            for i in range(resultLen):
                currentResult = result[i]
                currentExpected = expected[i]

                self.assertEqual(currentResult[0], currentExpected[0])
                self.assertEqual(currentResult[2], currentExpected[2])

                expectedQuery = currentExpected[1]
                if (isinstance(expectedQuery, Boolean)):
                    expectedQuery = simplify(expectedQuery)

                self.compareQuery(currentResult[1], expectedQuery)

    def test_differentIniGraphsMultiStates_contentAndQueryProcessed(self):
        vars = {"x": Symbol("$x$"),
                "i": Symbol("$i$")}
        result = []
        states = 3
        process = lambda data: result.append((data.part, data.query, data.sectionName, data.state)) 

        tests = [
                 [{}, [], []],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], [(0, True, "root", 1), (0, True, "root", 2), (0, True, "root", 3)]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                           FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, vars = vars),
                                            FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                                           FRB.IfPredPart("else", FRB.IfPredPartType.Else, vars = vars),
                                            FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, vars = vars)])}, ["root"],

                                           [(0, True, "root", 1),
                                            (0, True, "root", 2),
                                            (2, Eq(vars["i"], 0), "root", 1),
                                            (2, Eq(vars["i"], 0), "root", 2),
                                            (4, And(Not(Eq(vars["i"], 0)), True), "root", 1),
                                            (4, And(Not(Eq(vars["i"], 0)), True), "root", 2),
                                            (0, True, "root", 3),
                                            (2, Eq(vars["i"], 0), "root", 3),
                                            (4, And(Not(Eq(vars["i"], 0)), True), "root", 3),]],

                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                           FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x + 7 == 5", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfPredPart("if $x - 7 == 2", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["root"],
                                           
                                           [(0, True, "root", 1),
                                            (0, True, "root", 2),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "root", 1),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "root", 2),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "root", 1),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "root", 2),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "root", 1),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "root", 2),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "root", 1),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "root", 2),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "root", 1),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "root", 2),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "root", 1),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "root", 2),
                                            (20, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] + 7, 5)), "root", 1),
                                            (20, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] + 7, 5)), "root", 2),
                                            (23, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] - 7, 2)), "root", 1),
                                            (23, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] - 7, 2)), "root", 2),
                                            
                                            (0, True, "root", 3),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "root", 3),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "root", 3),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "root", 3),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "root", 3),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "root", 3),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "root", 3),
                                            (20, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] + 7, 5)), "root", 3),
                                            (23, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] - 7, 2)), "root", 3)]],
                                            
                 [{"loop": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                           FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                                                FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 2),
                                                FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["loop"],
                                           
                                           [(0, True, "loop", 1),
                                            (0, True, "loop", 2),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "loop", 1),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "loop", 2),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "loop", 1),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "loop", 2),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "loop", 1),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "loop", 2),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "loop", 1),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "loop", 2),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "loop", 1),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "loop", 2),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "loop", 1),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "loop", 2),
                                            
                                            (0, True, "loop", 3),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "loop", 3),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "loop", 3),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "loop", 3),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "loop", 3),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "loop", 3),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "loop", 3)]],
                                            
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, ".")], "cacheType": []}, 0)])},
                    [".a", ".b"],
                    
                    [(1, Eq(vars["x"] / 3, 2), ".a", 1),
                     (0, Eq(vars["x"] / 3, 2), ".aa", 1),
                     (0, Eq(vars["x"] / 3, 2), ".aa", 2),
                     (1, Eq(vars["x"] / 3, 2), ".a", 2),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a", 1),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab", 1),
                     (1, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Eq(vars["x"] + 3, 5)), ".", 1),
                     (1, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Eq(vars["x"] + 3, 5)), ".", 2),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".", 1),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".b", 1),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".b", 2),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".", 2),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab", 2),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a", 2),
                     
                     (1, Eq(vars["x"] / 3, 2), ".a", 3),
                     (0, Eq(vars["x"] / 3, 2), ".aa", 3),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a", 3),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab", 3),
                     (1, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Eq(vars["x"] + 3, 5)), ".", 3),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".", 3),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".b", 3)]],
                     
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                    [".b", ".a"],
                    
                    [(0, True, ".b", 1),
                     (0, True, ".b", 2),
                     (1, Eq(vars["x"] / 3, 2), ".a", 1),
                     (0, Eq(vars["x"] / 3, 2), ".aa", 1),
                     (0, Eq(vars["x"] / 3, 2), ".aa", 2),
                     (1, Eq(vars["x"] / 3, 2), ".a", 2),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a", 1),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab", 1),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab", 2),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a", 2),
                     
                     (0, True, ".b", 3),
                     (1, Eq(vars["x"] / 3, 2), ".a", 3),
                     (0, Eq(vars["x"] / 3, 2), ".aa", 3),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a", 3),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab", 3)]]
                     ]

        for test in tests:
            sections = test[0]
            targetSectionNames = test[1]

            graph = FRB.IniSectionGraph(sections, targetSectionNames)

            expected = test[2]
            expected = list(map(lambda processLine: (sections[processLine[2]].parts[processLine[0]], processLine[1], processLine[2], processLine[3]), expected))

            result.clear()
            graph.processIfContentByQuery(process, simplify = True, states = states)

            resultLen = len(result)
            self.assertEqual(resultLen, len(expected))

            for i in range(resultLen):
                currentResult = result[i]
                currentExpected = expected[i]

                self.assertEqual(currentResult[0], currentExpected[0])
                self.assertEqual(currentResult[2], currentExpected[2])
                self.assertEqual(currentResult[3], currentExpected[3])

                expectedQuery = currentExpected[1]
                if (isinstance(expectedQuery, Boolean)):
                    expectedQuery = simplify(expectedQuery)

                self.compareQuery(currentResult[1], expectedQuery)


    def test_differentIniGraphs1StateTrackKeys_keysTracked(self):
        vars = {"x": Symbol("$x$"),
                "i": Symbol("$i$")}
        result = []
        process = lambda data: result.append((data.part, data.query, data.sectionName, copy.deepcopy(data.colouring))) 

        tests = [
                 [{}, [], set(), []],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({}, 0)])}, ["root"], set(), [(0, True, "root", FRB.IfContentPartColouring({}))]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], set(), [(0, True, "root", FRB.IfContentPartColouring({}))]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], {"boo"}, [(0, True, "root", FRB.IfContentPartColouring({}))]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], {"boo", "cacheType", "S3FIFOVariant"}, [(0, True, "root", FRB.IfContentPartColouring({'S3FIFOVariant': [(0, '4')]}))]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                           FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, vars = vars),
                                            FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                                           FRB.IfPredPart("else", FRB.IfPredPartType.Else, vars = vars),
                                            FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, vars = vars)])}, ["root"], {"InductionType", "inductiveHypothesis"},

                                           [(0, True, "root", {'InductionType': [(0, 'Strong Induction (POSI)')], 'inductiveHypothesis': [(1, 'false')]}),
                                            (2, Eq(vars["i"], 0), "root", {'InductionType': 'Strong Induction (POSI)', 'inductiveHypothesis': 'false'}),
                                            (4, And(Not(Eq(vars["i"], 0)), True), "root", {'InductionType': 'Strong Induction (POSI)', 'inductiveHypothesis': [(0, 'true')]})]],

                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                           FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, vars = vars),
                                            FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                                           FRB.IfPredPart("else", FRB.IfPredPartType.Else, vars = vars),
                                            FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, vars = vars)])}, ["root"], set(),

                                           [(0, True, "root", {}),
                                            (2, Eq(vars["i"], 0), "root", {}),
                                            (4, And(Not(Eq(vars["i"], 0)), True), "root", {})]],

                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                           FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x + 7 == 5", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfPredPart("if $x - 7 == 2", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["root"], None,
                                           
                                           [(0, True, "root", {'a': [(0, '1')], 'b': [(1, '2')]}),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "root", {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "root", {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "root", {'a': [(0, '1')], 'b': [(1, '2')]}),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "root", {'a': [(0, '1')], 'b': [(1, '2')]}),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "root", {'a': [(0, '1')], 'b': [(1, '2')]}),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "root", {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}),
                                            (20, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] + 7, 5)), "root", {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}),
                                            (23, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] - 7, 2)), "root", {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]})]],
                                            
                 [{"loop": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                           FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                                                FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 2),
                                                FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["loop"], {"bello", "the end", "a", "b"},
                                           
                                           [(0, True, "loop", {'a': [(0, '1')], 'b': [(1, '2')]}),
                                            (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "loop", {'a': '1', 'b': [(1, '2')]}),
                                            (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "loop", {'a': '1', 'b': [(1, '2')]}),
                                            (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "loop", {'a': [(0, '1')], 'b': [(1, '2')]}),
                                            (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "loop", {'a': [(0, '1')], 'b': [(1, '2')]}),
                                            (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "loop", {'a': [(0, '1')], 'b': [(1, '2')]}),
                                            (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "loop", {'a': '1', 'b': [(1, '2')]})]],
                                            
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, ".")], "cacheType": []}, 0)])},
                    [".a", ".b"], {"run"},
                    
                    [(1, Eq(vars["x"] / 3, 2), ".a", {'run': [(0, '.aa')]}),
                     (0, Eq(vars["x"] / 3, 2), ".aa", {'run': '.aa'}),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a", {'run': [(0, '.ab')]}),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab", {'run': [(0, '.')]}),
                     (1, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Eq(vars["x"] + 3, 5)), ".", {'run': [(0, '.a')]}),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".", {'run': [(0, '.b')]}),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".b", {'run': '.b'})]],
                     
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                    [".b", ".a"], None,
                    
                    [(0, True, ".b", {'vcdim': [(0, '2')]}),
                     (1, Eq(vars["x"] / 3, 2), ".a", {'vcdim': '2', 'run': [(0, '.aa')]}),
                     (0, Eq(vars["x"] / 3, 2), ".aa", {'vcdim': [(0, '2')], 'run': '.aa'}   ),
                     (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a", {'vcdim': '2', 'run': [(0, '.ab')]}),
                     (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab", {'vcdim': [(0, '2')], 'run': '.ab'})]]
                     ]
        
        for test in tests:
            sections = test[0]
            targetSectionNames = test[1]
            keysToTrack = test[2]

            graph = FRB.IniSectionGraph(sections, targetSectionNames)

            expected = test[3]
            expected = list(map(lambda processLine: (sections[processLine[2]].parts[processLine[0]], processLine[1], processLine[2], processLine[3]), expected))

            result.clear()
            graph.processIfContentByQuery(process, simplify = True, colour = True, colourKeys = keysToTrack)

            resultLen = len(result)
            self.assertEqual(resultLen, len(expected))


            for i in range(resultLen):
                currentResult = result[i]
                currentExpected = expected[i]

                self.assertEqual(currentResult[0], currentExpected[0])
                self.assertEqual(currentResult[2], currentExpected[2])

                expectedQuery = currentExpected[1]
                if (isinstance(expectedQuery, Boolean)):
                    expectedQuery = simplify(expectedQuery)

                self.compareQuery(currentResult[1], expectedQuery)
                self.compareIfContentPartColouring(currentResult[3], currentExpected[3])

    # ========================================================
    # ====================== rename ==========================

    def test_differentRenameFuncs_sectionsRenamed(self):
        tests = [[{}, [], lambda old: f"new{old}", set(), {}, []],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], lambda old: f"new{old}", {"newroot"}, {}, ["newroot"]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], lambda old: old, {"root"}, {}, ["root"]],
                 [{"loop": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0),
                                           FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                           FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "loop")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)])},
                  ["loop"], lambda old: f"new{old}", {"newloop"}, {"newloop": ["newloop"]}, ["newloop"]],
                [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                    [".", ".a"], lambda old: f"new{old}" if (len(old) >= 3) else old, 
                    {".", ".a", ".b", "new.aa", "new.ab"}, {".": [".a", ".b"], ".a": ["new.aa", "new.ab"]}, ["."]],

                [{".": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                    [".", ".a"], lambda old: f"new" if (len(old) >= 3) else old, 
                    {".", ".a", ".b", "new"}, {".": [".a", ".b"], ".a": ["new"]}, ["."]],
                    
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, ".ba")]}, 0)]),
                    ".ba": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                    [".aa", ".ab", ".ba", ".b", ".a"], lambda old: "new", 
                    {"new"}, {"new": ["new"]}, ["new"]]]

        for test in tests:
            sections = test[0]
            targetSectionNames = test[1]
            renameFunc = test[2]

            graph = FRB.IniSectionGraph(sections, targetSectionNames)
            graph.rename(renameFunc)

            expectedSectionNames = test[3]
            expectedNeighbours = test[4]
            expectedRoots = test[5]

            self.compareSet(set(graph.sections.keys()), expectedSectionNames)
            self.compareList(graph.roots, expectedRoots)
            self.compareDict(graph.neighbours, expectedNeighbours, lambda resVal, expVal: self.compareList(resVal, expVal))

    # ========================================================
    # ============= iterSectsByContentPart ===================

    def test_differentSections_ifContentIteratedWithColour(self):
        tests = [
                 [{}, [], None, []],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], None,[("root", ("root", 0), 1, FRB.IfContentPartColouring({"S3FIFOVariant": [(0, "4")]}))]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                           FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, vars = vars),
                                            FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                                           FRB.IfPredPart("else", FRB.IfPredPartType.Else, vars = vars),
                                            FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, vars = vars)])}, ["root"], None,

                                           [['root', ("root", 0), 1, {'InductionType': [(0, 'Strong Induction (POSI)')], 'inductiveHypothesis': [(1, 'false')]}],
                                            ['root', ("root", 2), 1, {'InductionType': 'Strong Induction (POSI)', 'inductiveHypothesis': 'false', 'baseCase': [(0, '0')]}],
                                            ['root', ("root", 4), 1, {'InductionType': 'Strong Induction (POSI)', 'inductiveHypothesis': [(0, 'true')], 'inductiveStep': [(1, 'true')]}]]],

                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                           FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, vars = vars),
                                            FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                                           FRB.IfPredPart("else", FRB.IfPredPartType.Else, vars = vars),
                                            FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, vars = vars)])}, ["root"], {"inductiveHypothesis"},

                                           [['root', ("root", 0), 1, {'inductiveHypothesis': [(1, 'false')]}],
                                            ['root', ("root", 2), 1, {'inductiveHypothesis': 'false'}],
                                            ['root', ("root", 4), 1, {'inductiveHypothesis': [(0, 'true')]}]]],

                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                           FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("if $x + 7 == 5", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfPredPart("if $x - 7 == 2", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["root"], None,
                                           
                                           [['root', ("root", 0), 1, {'a': [(0, '1')], 'b': [(1, '2')]}],
                                            ['root', ("root", 3), 1, {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}],
                                            ['root', ("root", 7), 1, {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}],
                                            ['root', ("root", 9), 1, {'a': [(0, '1')], 'b': [(1, '2')]}],
                                            ['root', ("root", 14), 1, {'a': [(0, '1')], 'b': [(1, '2')]}],
                                            ['root', ("root", 16), 1, {'a': [(0, '1')], 'b': [(1, '2')]}],
                                            ['root', ("root", 18), 1, {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}],
                                            ['root', ("root", 20), 1, {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}],
                                            ['root', ("root", 23), 1, {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}]]],
                                            
                 [{"loop": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                                           FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                                                    FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                                                FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                           FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 2),
                                                FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                                                FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                                                FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                                                FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["loop"], None,
                                           
                                           [['loop', ("loop", 0), 1, {'a': [(0, '1')], 'b': [(1, '2')]}],
                                            ['loop', ("loop", 3), 1, {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}],
                                            ['loop', ("loop", 7), 1, {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')]}],
                                            ['loop', ("loop", 9), 1, {'a': [(0, '1')], 'b': [(1, '2')], 'run': [(2, 'loop')]}],
                                            ['loop', ("loop", 14), 1, {'a': [(0, '1')], 'b': [(1, '2')], 'run': [(2, 'loop')]}],
                                            ['loop', ("loop", 16), 1, {'a': [(0, '1')], 'b': [(1, '2')]}],
                                            ['loop', ("loop", 18), 1, {'a': '1', 'b': [(1, '2')], 'target': [(0, '1')], 'run': [(2, 'loop')]}],]],
                                            
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, ".")], "cacheType": []}, 0)])},
                    [".a", ".b"], None,
                    
                    [['.a', (".a", 1), 1, {'run': [(0, '.aa')]}],
                    ['.aa', (".aa", 0), 1, {'run': '.aa', 'vcdim': [(0, '2')]}],
                    ['.a', (".a", 3), 1, {'run': [(0, '.ab')]}],
                    ['.ab', (".ab", 0), 1, {'run': [(0, '.')]}],
                    ['.', (".", 1), 1, {'run': [(0, '.a')]}],
                    ['.', (".", 3), 1, {'run': [(0, '.b')]}],
                    ['.b', (".b", 0), 1, {'run': '.b', 'vcdim': [(0, '2')]}],]],
                     
                 [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                                        FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                                        FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                                        FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                                        FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                                          FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                                          FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                                          FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                                          FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                    ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                    [".b", ".a"], None,
                    
                    [['.b', (".b", 0), 1, {'vcdim': [(0, '2')]}],
                     ['.a', (".a", 1), 1, {'run': [(0, '.aa')]}],
                     ['.aa', (".aa", 0), 1, {'run': '.aa', 'vcdim': [(0, '2')]}],
                     ['.a', (".a", 3), 1, {'run': [(0, '.ab')]}],
                     ['.ab', (".ab", 0), 1, {'run': '.ab', 'vcdim': [(0, '2')]}]]],

                  [{"top": FRB.IfTemplate([FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If),
                                           FRB.IfContentPart({"run": [(0, "left")], "topper": [(1, "a")]}, 1),
                                           FRB.IfPredPart("else", FRB.IfPredPartType.Else),
                                           FRB.IfContentPart({"run": [(0, "right")], "topper": [(1, "b")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                    "left": FRB.IfTemplate([FRB.IfContentPart({"lefter": [(0, "1")], "run": [(1, "bottom")]}, 0)]),
                    "right": FRB.IfTemplate([FRB.IfContentPart({"righter": [(0, "2")], "run": [(1, "bottom")]}, 0)]),
                    "bottom": FRB.IfTemplate([FRB.IfContentPart({"bottomer": [(0, "x")]}, 0)])}, ["top"], None,
                    
                    [['top', ("top", 1), 1, {'run': [(0, 'left')], 'topper': [(1, 'a')]}],
                     ['left', ("left", 0), 1, {'run': [(1, 'bottom')], 'topper': 'a', 'lefter': [(0, '1')]}],
                     ['bottom', ("bottom", 0), 1, {'run': 'bottom', 'topper': 'a', 'lefter': '1', 'bottomer': [(0, 'x')]}],
                     ['top', ("top", 3), 1, {'run': [(0, 'right')], 'topper': [(1, 'b')]}],
                     ['right', ("right", 0), 1, {'run': [(1, 'bottom')], 'topper': 'b', 'righter': [(0, '2')]}],
                     ['bottom', ("bottom", 0), 1, {'run': 'bottom', 'topper': 'b', 'righter': '2', 'bottomer': [(0, 'x')]}]]]
                    ]
        
        result = []
        states = 1

        for test in tests:
          sections = test[0]
          targetSectionNames = test[1]
          targetKeys = test[2]

          graph = FRB.IniSectionGraph(sections, targetSectionNames)

          expected = test[3]
          result.clear()

          for ifContentPartData in graph.iterByContentPart(states = states, colour = True, colourKeys = targetKeys):
               ifContentPartData.colouring = copy.deepcopy(ifContentPartData.colouring)
               result.append(ifContentPartData)

          resultLen = len(result)
          self.assertEqual(resultLen, len(expected))

          for i in range(resultLen):
               currentResult = result[i]
               currentResult = (currentResult.sectionName, currentResult.part, currentResult.state, currentResult.colouring)
               currentExpected = expected[i]

               currentExpPartKeys = currentExpected[1]
               currentExpPart = sections[currentExpPartKeys[0]].parts[currentExpPartKeys[1]]

               self.assertEqual(currentResult[0], currentExpected[0])
               self.assertEqual(currentResult[1], currentExpPart)
               self.assertEqual(currentResult[2], currentExpected[2])
               self.compareIfContentPartColouring(currentResult[3], currentExpected[3])

    # ====================== computeSectionPredecessors ===========================
    # NOTE: moved here (and off of RegSurroundedAdd, where it was originally developed as a private helper) since
    # "which IfContentPart must run immediately before which, within one section" is generic graph structure any
    # graph edit reasoning about ordering/dedup across run= calls could need, not something specific to that one
    # class -- see also buildPartPredecessorGraph/buildCallGraph below, and FRB.GraphTools for the generic,
    # non-.ini-specific graph algorithms (reachability, dataflow fixpoints) built alongside these

    def test_computeSectionPredecessors_noBranching_eachPartDependsOnlyOnThePreviousOne(self):
        a = FRB.IfContentPart({"a": [(0, "1")]})
        b = FRB.IfContentPart({"b": [(0, "2")]})
        predecessors = FRB.IniSectionGraph.computeSectionPredecessors([a, b])

        self.compareList(predecessors[id(a)], [])
        self.compareList(predecessors[id(b)], [id(a)])

    def test_computeSectionPredecessors_ifElse_eachBranchDependsOnlyOnWhatPrecededTheIf(self):
        a = FRB.IfContentPart({"a": [(0, "1")]})
        branch1 = FRB.IfContentPart({"vb1": [(0, "1")]})
        branch2 = FRB.IfContentPart({"vb1": [(0, "2")]})
        parts = [a, FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If), branch1,
                 FRB.IfPredPart("else", FRB.IfPredPartType.Else), branch2,
                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]
        predecessors = FRB.IniSectionGraph.computeSectionPredecessors(parts)

        self.compareList(predecessors[id(a)], [])
        self.compareList(predecessors[id(branch1)], [id(a)])
        self.compareList(predecessors[id(branch2)], [id(a)])

    def test_computeSectionPredecessors_afterEndIfWithElse_dependsOnEveryBranchOnly(self):
        branch1 = FRB.IfContentPart({"vb1": [(0, "1")]})
        branch2 = FRB.IfContentPart({"vb1": [(0, "2")]})
        after = FRB.IfContentPart({"c": [(0, "9")]})
        parts = [FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If), branch1,
                 FRB.IfPredPart("else", FRB.IfPredPartType.Else), branch2,
                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf), after]
        predecessors = FRB.IniSectionGraph.computeSectionPredecessors(parts)

        self.compareSet(set(predecessors[id(after)]), {id(branch1), id(branch2)})

    def test_computeSectionPredecessors_afterEndIfWithoutElse_alsoDependsOnWhatPrecededTheIf(self):
        a = FRB.IfContentPart({"a": [(0, "1")]})
        branch1 = FRB.IfContentPart({"vb1": [(0, "1")]})
        after = FRB.IfContentPart({"c": [(0, "9")]})
        parts = [a, FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If), branch1,
                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf), after]
        predecessors = FRB.IniSectionGraph.computeSectionPredecessors(parts)

        self.compareSet(set(predecessors[id(after)]), {id(a), id(branch1)})

    def test_computeSectionPredecessors_elif_eachBranchStillOnlyDependsOnWhatPrecededTheIf(self):
        a = FRB.IfContentPart({"a": [(0, "1")]})
        branch1 = FRB.IfContentPart({"vb1": [(0, "1")]})
        branch2 = FRB.IfContentPart({"vb1": [(0, "2")]})
        branch3 = FRB.IfContentPart({"vb1": [(0, "3")]})
        parts = [a, FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If), branch1,
                 FRB.IfPredPart("else if $x == 2", FRB.IfPredPartType.Elif), branch2,
                 FRB.IfPredPart("else", FRB.IfPredPartType.Else), branch3,
                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]
        predecessors = FRB.IniSectionGraph.computeSectionPredecessors(parts)

        self.compareList(predecessors[id(branch1)], [id(a)])
        self.compareList(predecessors[id(branch2)], [id(a)])
        self.compareList(predecessors[id(branch3)], [id(a)])

    # ====================== buildPartPredecessorGraph ===========================

    def test_buildPartPredecessorGraph_runCall_calleeEntryDependsOnTheCallingPart(self):
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])

        predecessors = graph.buildPartPredecessorGraph()
        parentPart = sections["parent"].parts[0]
        childPart = sections["child"].parts[0]

        self.compareList(predecessors[id(childPart)], [id(parentPart)])

    def test_buildPartPredecessorGraph_selfReferencingRunCall_doesNotHang(self):
        # a section that calls itself via "run =" -- its own part ends up listed as its own predecessor. This
        # doesn't cause a real problem for a dedup consumer (see RegSurroundedAdd's own _editEarliest/_editLatest
        # tests): the edge just never gets consulted, since a part is only ever visited (and decided) once
        sections = {"loop": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "loop")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["loop"])
        loopPart = sections["loop"].parts[0]

        predecessors = graph.buildPartPredecessorGraph()
        self.compareList(predecessors[id(loopPart)], [id(loopPart)])

    def test_buildPartPredecessorGraph_mutualRunCalls_bothDirectionsRecorded(self):
        sections = {
            "A": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "B")]}, 0)]),
            "B": FRB.IfTemplate([FRB.IfContentPart({"c": [(0, "2")], "run": [(1, "A")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["A"])
        partA = sections["A"].parts[0]
        partB = sections["B"].parts[0]

        predecessors = graph.buildPartPredecessorGraph()
        self.compareList(predecessors[id(partA)], [id(partB)])
        self.compareList(predecessors[id(partB)], [id(partA)])

    # ====================== buildCallGraph ===========================
    # NOTE: RegSurroundedAdd's own tests already exercise this extensively end-to-end (via its cyclic/multi-key
    # edit() tests); these focus narrowly on the graph shape itself -- the virtual ("exit", id(part)) node, and
    # which nodes count as call graph roots

    def test_buildCallGraph_partWithNoCalls_ownNodeServesAsBothEntryAndExit(self):
        sections = {"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["root"])
        part = sections["root"].parts[0]

        callGraph = graph.buildCallGraph()

        self.compareDict(callGraph.partsById, {id(part): part})
        self.compareSet(callGraph.rootNodeIds, {id(part)})
        self.assertFalse(("exit", id(part)) in callGraph.forwardEdges)
        self.assertFalse(("exit", id(part)) in callGraph.backwardEdges)
        self.assertEqual(callGraph.exitNodeOf(id(part)), id(part))

    def test_buildCallGraph_partWithACall_ownNodeLeadsIntoTheCalleeAndExitNodeLeadsToTheReturn(self):
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])
        parentPart = sections["parent"].parts[0]
        childPart = sections["child"].parts[0]

        callGraph = graph.buildCallGraph()

        # the calling part's own node leads into the callee's entry, NOT to a "continuation" node
        self.compareList(callGraph.forwardEdges[id(parentPart)], [id(childPart)])

        # since childPart has no further within-section successor, and its section has exactly one caller
        # (parentPart), it leads back to parentPart's own virtual exit node -- the "continue here once the call
        # has returned" point
        self.compareList(callGraph.forwardEdges[id(childPart)], [("exit", id(parentPart))])

        self.compareSet(callGraph.rootNodeIds, {id(parentPart)})
        self.assertEqual(callGraph.exitNodeOf(id(parentPart)), ("exit", id(parentPart)))
        self.assertEqual(callGraph.exitNodeOf(id(childPart)), id(childPart))

    def test_buildCallGraph_selfReferencingRunCall_ownNodeAndExitNodeAreSeparateSelfLoops(self):
        # an unconditional, inescapable self-call -- the "before the call" node (id(part)) and the "after the
        # call returns" node (("exit", id(part))) end up as two SEPARATE self-loops, disconnected from each
        # other, since the call never actually returns. This is exactly the structural gap that made a naive
        # dataflow fixpoint unsound for this case (see RegSurroundedAdd's cyclic edit() tests / GraphTools) --
        # id(part) is a genuine root (reachable), while ("exit", id(part)) is not reachable from it at all
        sections = {"loop": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "loop")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["loop"])
        part = sections["loop"].parts[0]

        callGraph = graph.buildCallGraph()

        self.compareList(callGraph.forwardEdges[id(part)], [id(part)])
        self.compareList(callGraph.forwardEdges[("exit", id(part))], [("exit", id(part))])
        self.compareSet(callGraph.rootNodeIds, {id(part)})
        self.assertEqual(callGraph.exitNodeOf(id(part)), ("exit", id(part)))

        reachable = FRB.GraphTools.getReachableNodes(callGraph.forwardEdges, callGraph.rootNodeIds)
        self.assertTrue(id(part) in reachable)
        self.assertFalse(("exit", id(part)) in reachable)

    # ====================== GraphTools ===========================
    # NOTE: these are deliberately generic (plain str node ids, no IfContentPart/section involved at all) -- they
    # exercise FRB.GraphTools' own contract directly, decoupled from anything .ini-specific. RegSurroundedAdd's
    # own cyclic edit() tests already cover the tools end-to-end via buildCallGraph's real node shapes

    def test_graphTools_getReachableNodes_onlyNodesOnSomePathFromRootsAreIncluded(self):
        forwardEdges = {"root": ["a"], "a": ["b"], "island": ["other"]}
        reachable = FRB.GraphTools.getReachableNodes(forwardEdges, {"root"})

        self.compareSet(reachable, {"root", "a", "b"})

    def test_graphTools_getReachableNodes_cycleTerminates(self):
        forwardEdges = {"root": ["a"], "a": ["b"], "b": ["a"]}
        reachable = FRB.GraphTools.getReachableNodes(forwardEdges, {"root"})

        self.compareSet(reachable, {"root", "a", "b"})

    def test_graphTools_clampFactsToReachable_unreachableNodeForcedFalse(self):
        result = FRB.GraphTools.clampFactsToReachable({"a": True, "b": True}, {"a"})
        self.compareDict(result, {"a": True, "b": False})

    def test_graphTools_runForwardMustFixpoint_linearChain_propagatesForward(self):
        # root (untouched) -> a (defines the property) -> b (untouched) -- satisfied entering "a"? No (root is
        # forced False). Satisfied entering "b"? Yes (carried forward from "a")
        forwardEdges = {"root": ["a"], "a": ["b"]}
        backwardEdges = {"a": ["root"], "b": ["a"]}
        localFacts = {"a": (True, True)}

        result = FRB.GraphTools.runForwardMustFixpoint(forwardEdges, backwardEdges, {"root"}, localFacts)

        self.assertFalse(result["root"])
        self.assertTrue(result["b"])

    def test_graphTools_runForwardMustFixpoint_selfLoopNeverEstablishesTheProperty_staysUnsatisfied(self):
        # root, self-looping, never itself establishes the property -- must stay unsatisfied forever, not get
        # stuck on the optimistic starting value
        forwardEdges = {"root": ["root"]}
        backwardEdges = {"root": ["root"]}
        localFacts = {}

        result = FRB.GraphTools.runForwardMustFixpoint(forwardEdges, backwardEdges, {"root"}, localFacts)

        self.assertFalse(result["root"])

    def test_graphTools_runBackwardMustFixpoint_linearChain_propagatesBackward(self):
        # a (untouched) -> b (establishes the property) -- guaranteed after "a" exits? Yes. Guaranteed after "b"
        # exits? No (b is a terminal, forced False)
        forwardEdges = {"a": ["b"]}
        backwardEdges = {"b": ["a"]}
        localFacts = {"b": True}

        result = FRB.GraphTools.runBackwardMustFixpoint(forwardEdges, backwardEdges, localFacts)

        self.assertTrue(result["a"])
        self.assertFalse(result["b"])

    def test_graphTools_runBackwardMustFixpoint_cycleWhereOnlyOneNodeEstablishesTheProperty_allNodesGuaranteed(self):
        # a -> b -> a, "b" establishes the property -- guaranteed after "a" exits (loops back into "b"), and
        # guaranteed after "b" exits too (loops back into itself, by way of "a")
        forwardEdges = {"a": ["b"], "b": ["a"]}
        backwardEdges = {"b": ["a"], "a": ["b"]}
        localFacts = {"b": True}

        result = FRB.GraphTools.runBackwardMustFixpoint(forwardEdges, backwardEdges, localFacts)

        self.assertTrue(result["a"])
        self.assertTrue(result["b"])

    # ========================================================