import sys
from sympy import Symbol, And, Eq, Not, Ne, simplify
from sympy.logic.boolalg import Boolean
from typing import Dict, Any, Union

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB

Part = Union[str, Dict[str, Any]]


class IniSectionGraphTest(BaseUnitTest):

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
        process = lambda data: result.append((data.part, data.query, data.sectionName, data.kvps)) 

        tests = [
                 [{}, [], {}, []],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({}, 0)])}, ["root"], set(), [(0, True, "root", {})]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], set(), [(0, True, "root", {})]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], {"boo"}, [(0, True, "root", {})]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)])}, ["root"], {"boo", "cacheType", "S3FIFOVariant"}, [(0, True, "root", {"cacheType": [], "S3FIFOVariant": [(0, "4")]})]],
                 [{"root": FRB.IfTemplate([FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")],
                                                                            "inductiveHypothesis": [(1, "false")]}, 0),
                                           FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, vars = vars),
                                            FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                                           FRB.IfPredPart("else", FRB.IfPredPartType.Else, vars = vars),
                                            FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, vars = vars)])}, ["root"], {"InductionType", "inductiveHypothesis"},

                                           [(0, True, "root", {"InductionType": [(0, "Strong Induction (POSI)")], "inductiveHypothesis": [(1, "false")]}),
                                            (2, Eq(vars["i"], 0), "root", {"InductionType": [(0, "Strong Induction (POSI)")], "inductiveHypothesis": [(1, "false")]}),
                                            (4, And(Not(Eq(vars["i"], 0)), True), "root", {"InductionType": [(0, "Strong Induction (POSI)")], "inductiveHypothesis": [(0, "true")]})]],

                #  [{"root": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                #                            FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                #                                 FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                #                                     FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                            FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                #                                 FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                #                                 FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                            FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                #                                 FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                            FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                #                                 FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                #                                 FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                #                                 FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                #                                 FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                                 FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                #                                 FRB.IfPredPart("if $x + 7 == 5", FRB.IfPredPartType.If),
                #                                 FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                                 FRB.IfPredPart("if $x - 7 == 2", FRB.IfPredPartType.If),
                #                                 FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["root"],
                                           
                #                            [(0, True, "root"),
                #                             (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "root"),
                #                             (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "root"),
                #                             (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "root"),
                #                             (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "root"),
                #                             (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "root"),
                #                             (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "root"),
                #                             (20, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] + 7, 5)), "root"),
                #                             (23, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] - 7, 2)), "root")]],
                                            
                #  [{"loop": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                #                            FRB.IfPredPart("if $x * 6 == 0", FRB.IfPredPartType.If),
                #                                 FRB.IfPredPart("if $x - 3 == 0", FRB.IfPredPartType.If),
                #                                     FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                            FRB.IfPredPart("else if $x / 5 == 0", FRB.IfPredPartType.Elif),
                #                                 FRB.IfPredPart("if $x + 6 == 0", FRB.IfPredPartType.If),
                #                                 FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                            FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                #                                 FRB.IfPredPart("if $x * 7 == 0", FRB.IfPredPartType.If),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                            FRB.IfPredPart("else if $x - 10 == 0", FRB.IfPredPartType.Elif),
                #                                 FRB.IfPredPart("if $x * 3 == 0", FRB.IfPredPartType.If),
                #                                 FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 2),
                #                                 FRB.IfPredPart("else if $x + 3 == 0", FRB.IfPredPartType.Elif),
                #                                 FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                #                                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf),
                #                                 FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")], "run": [(2, "loop")]}, 1),
                #                            FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)], treeCls = FRB.IfTemplateTree)}, ["loop"],
                                           
                #                            [(0, True, "loop"),
                #                             (3, And(Eq(vars["x"] * 6, 0), Eq(vars["x"] - 3, 0)), "loop"),
                #                             (7, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0), Eq(vars["x"] + 6, 0)), "loop"),
                #                             (9, And(Not(Eq(vars["x"] * 6, 0)), Eq(vars["x"] / 5, 0)), "loop"),
                #                             (14, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Eq(vars["x"] * 3, 0)), "loop"),
                #                             (16, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0), Not(Eq(vars["x"] * 3, 0)), Eq(vars["x"] + 3, 0)), "loop"),
                #                             (18, And(Not(Eq(vars["x"] * 6, 0)), Not(Eq(vars["x"] / 5, 0)), Eq(vars["x"] - 10, 0)), "loop")]],
                                            
                #  [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                #                         FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                #                         FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                #                         FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                #                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                #     ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                #                           FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                #                           FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                #                           FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                #                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                #     ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                #     ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                #     ".ab": FRB.IfTemplate([FRB.IfContentPart({"run": [(0, ".")], "cacheType": []}, 0)])},
                #     [".a", ".b"],
                    
                #     [(1, Eq(vars["x"] / 3, 2), ".a"),
                #      (0, Eq(vars["x"] / 3, 2), ".aa"),
                #      (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a"),
                #      (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab"),
                #      (1, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Eq(vars["x"] + 3, 5)), "."),
                #      (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), "."),
                #      (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11), Ne(vars["x"] + 3, 5), Eq(vars["x"] * 5, 10)), ".b")]],
                     
                #  [{".": FRB.IfTemplate([FRB.IfPredPart("if $x + 3 == 5", FRB.IfPredPartType.If),
                #                         FRB.IfContentPart({"run": [(0, ".a")], "cacheType": []}, 1),
                #                         FRB.IfPredPart("else if $x * 5 == 10", FRB.IfPredPartType.Elif),
                #                         FRB.IfContentPart({"run": [(0, ".b")], "cacheType": []}, 1),
                #                         FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                #     ".a": FRB.IfTemplate([FRB.IfPredPart("if $x / 3 == 2", FRB.IfPredPartType.If),
                #                           FRB.IfContentPart({"run": [(0, ".aa")], "cacheType": []}, 1),
                #                           FRB.IfPredPart("else if $x - 5 == 11", FRB.IfPredPartType.Elif),
                #                           FRB.IfContentPart({"run": [(0, ".ab")], "cacheType": []}, 1),
                #                           FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf)]),
                #     ".b": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                #     ".aa": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)]),
                #     ".ab": FRB.IfTemplate([FRB.IfContentPart({"vcdim": [(0, "2")], "cacheType": []}, 0)])},
                #     [".b", ".a"],
                    
                #     [(0, True, ".b"),
                #      (1, Eq(vars["x"] / 3, 2), ".a"),
                #      (0, Eq(vars["x"] / 3, 2), ".aa"),
                #      (3, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".a"),
                #      (0, And(Ne(vars["x"] / 3, 2), Eq(vars["x"] - 5, 11)), ".ab")]]
                     ]
        
        for test in tests:
            print(f"======== TEST ===============")
            sections = test[0]
            targetSectionNames = test[1]
            keysToTrack = test[2]

            graph = FRB.IniSectionGraph(sections, targetSectionNames)

            expected = test[3]
            expected = list(map(lambda processLine: (sections[processLine[2]].parts[processLine[0]], processLine[1], processLine[2], processLine[3]), expected))

            result.clear()
            graph.processIfContentByQuery(process, simplify = True, keysToTrack = keysToTrack)

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
                self.compareDict(currentResult[3], currentExpected[3], compareValues = lambda resVals, expectedVals: self.compareList(resVals, expectedVals, compareValues = lambda resVal, expectedVal: self.compareList(resVal, expectedVal)))

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