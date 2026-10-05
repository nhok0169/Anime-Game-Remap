##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits

##### ExtImports
import sys
##### EndExtImports

##### LocalImports
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
from src.py.FixRaidenBoss2 import GraphTools
##### EndLocalImports


# GraphTools is generic (plain Dict[node, List[node]] adjacency, no notion of IfContentPart/section
# at all) -- these tests build small graphs directly out of plain strings, not through
# IniSectionGraph, matching how the tools are actually meant to be used/tested in isolation.
#
# GraphTools is C++-backed (see AI Agent Help/IniGraphEditing/CLAUDE.md) -- this file exercises it
# entirely through its public static methods, which is all a caller (whether pure Python or C++)
# can see. The pure-Python original (tools/GraphTools.py) has been deleted outright.
class GraphToolsTest(BaseUnitTest):
    # ========= getReachableNodes ====================================

    def test_getReachableNodes_onlyNodesOnSomePathFromRootsAreIncluded(self):
        forwardEdges = {"a": ["b"], "b": ["c"], "island": ["other"]}
        result = GraphTools.getReachableNodes(forwardEdges, {"a"})
        self.compareSet(result, {"a", "b", "c"})

    def test_getReachableNodes_cycleTerminates(self):
        forwardEdges = {"a": ["b"], "b": ["a"]}
        result = GraphTools.getReachableNodes(forwardEdges, {"a"})
        self.compareSet(result, {"a", "b"})

    # ========================================================
    # ========= clampFactsToReachable =========================

    def test_clampFactsToReachable_unreachableNodeForcedFalse(self):
        facts = {"a": True, "island": True}
        reachable = {"a"}
        result = GraphTools.clampFactsToReachable(facts, reachable)
        self.compareDict(result, {"a": True, "island": False})

    def test_clampFactsToReachable_reachableFalseStaysFalse(self):
        facts = {"a": False}
        reachable = {"a"}
        result = GraphTools.clampFactsToReachable(facts, reachable)
        self.compareDict(result, {"a": False})

    # ========================================================
    # ========= runForwardMustFixpoint =========================

    def test_runForwardMustFixpoint_linearChain_propagatesForward(self):
        # a -> b -> c; "a" itself establishes the property by the time it's done
        forwardEdges = {"a": ["b"], "b": ["c"]}
        backwardEdges = {"b": ["a"], "c": ["b"]}
        localFacts = {"a": (True, True)}

        result = GraphTools.runForwardMustFixpoint(forwardEdges, backwardEdges, {"a"}, localFacts)

        self.assertFalse(result["a"])  # not yet satisfied entering the root
        self.assertTrue(result["b"])   # satisfied entering b (a already ran)
        self.assertTrue(result["c"])   # still satisfied entering c

    def test_runForwardMustFixpoint_selfLoopNeverEstablishesTheProperty_staysUnsatisfied(self):
        # a self-loop with no local fact anywhere never establishes the property, no matter how
        # many times it loops back around
        forwardEdges = {"a": ["a"]}
        backwardEdges = {"a": ["a"]}
        localFacts = {}

        result = GraphTools.runForwardMustFixpoint(forwardEdges, backwardEdges, {"a"}, localFacts)
        self.assertFalse(result["a"])

    # ========================================================
    # ========= runBackwardMustFixpoint =========================

    def test_runBackwardMustFixpoint_linearChain_propagatesBackward(self):
        # a -> b -> c; "c" itself establishes the property
        forwardEdges = {"a": ["b"], "b": ["c"]}
        backwardEdges = {"b": ["a"], "c": ["b"]}
        localFacts = {"c": True}

        result = GraphTools.runBackwardMustFixpoint(forwardEdges, backwardEdges, localFacts)

        self.assertTrue(result["a"])    # guaranteed somewhere after "a" exits (via b -> c)
        self.assertTrue(result["b"])    # guaranteed somewhere after "b" exits (c itself)
        self.assertFalse(result["c"])   # "guaranteed after c exits" is strictly *after* c -- c is a
                                         # dead end (no outgoing edges), so nothing runs after it at
                                         # all; c's own local fact only feeds forward into b/a above,
                                         # not into its own reported exit fact

    def test_runBackwardMustFixpoint_cycleWhereOnlyOneNodeEstablishesTheProperty_allNodesGuaranteed(self):
        # a -> b -> a; only "b" ever establishes the property, but since the cycle always reaches
        # "b" again, both nodes end up guaranteed
        forwardEdges = {"a": ["b"], "b": ["a"]}
        backwardEdges = {"a": ["b"], "b": ["a"]}
        localFacts = {"b": True}

        result = GraphTools.runBackwardMustFixpoint(forwardEdges, backwardEdges, localFacts)

        self.assertTrue(result["a"])
        self.assertTrue(result["b"])

    def test_runBackwardMustFixpoint_deadEndNoLocalFact_unsatisfied(self):
        # a genuine dead end (no outgoing edges) with no local fact of its own can never guarantee
        # the property
        forwardEdges = {}
        backwardEdges = {}
        localFacts = {"deadEnd": False}

        result = GraphTools.runBackwardMustFixpoint(forwardEdges, backwardEdges, localFacts)
        self.assertFalse(result["deadEnd"])
