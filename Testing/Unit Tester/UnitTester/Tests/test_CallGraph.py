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
##### EndLocalImports


class CallGraphTest(BaseUnitTest):
    # "parent" calls "child" via 'run ='; "child" makes no call of its own
    def makeParentChildGraph(self):
        sections = {
            "parent": FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")], "run": [(1, "child")]}, 0)]),
            "child": FRB.IfTemplate([FRB.IfContentPart({"b": [(0, "2")]}, 0)]),
        }
        graph = FRB.IniSectionGraph(sections, ["parent"])
        parentPart = sections["parent"].parts[0]
        childPart = sections["child"].parts[0]
        return graph, parentPart, childPart

    def test_buildCallGraph_noCalls_selfEdgesOnly(self):
        sections = {"a": FRB.IfTemplate([FRB.IfContentPart({"x": [(0, "1")]}, 0)])}
        graph = FRB.IniSectionGraph(sections, ["a"])
        part = sections["a"].parts[0]

        callGraph = graph.buildCallGraph()
        self.assertEqual(callGraph.partsById, {id(part): part})
        self.assertEqual(callGraph.rootNodeIds, {id(part)})
        # a part that makes no call has no successors and no predecessors within its own section
        self.assertEqual(callGraph.forwardEdges, {})
        self.assertEqual(callGraph.backwardEdges, {})

    def test_buildCallGraph_parentCallsChild_edgesReflectTheCall(self):
        graph, parentPart, childPart = self.makeParentChildGraph()
        callGraph = graph.buildCallGraph()

        self.assertEqual(callGraph.partsById, {id(parentPart): parentPart, id(childPart): childPart})
        self.assertEqual(callGraph.rootNodeIds, {id(parentPart)})

        # parent -> child (the call itself); child -> exit(parent) (control returns to parent's
        # continuation once child, which makes no further call, finishes)
        self.assertEqual(callGraph.forwardEdges, {
            id(parentPart): [id(childPart)],
            id(childPart): [("exit", id(parentPart))],
        })
        self.assertEqual(callGraph.backwardEdges, {
            id(childPart): [id(parentPart)],
            ("exit", id(parentPart)): [id(childPart)],
        })

    def test_exitNodeOf_partThatCalls_returnsExitTuple(self):
        graph, parentPart, childPart = self.makeParentChildGraph()
        callGraph = graph.buildCallGraph()
        self.assertEqual(callGraph.exitNodeOf(id(parentPart)), ("exit", id(parentPart)))

    def test_exitNodeOf_partThatDoesNotCall_returnsItself(self):
        graph, parentPart, childPart = self.makeParentChildGraph()
        callGraph = graph.buildCallGraph()
        self.assertEqual(callGraph.exitNodeOf(id(childPart)), id(childPart))

    # See the port's own plan on why this correlation is load-bearing (RegSurroundedAdd.py/
    # RegFillMissing.py key their own dataflow facts off id(part) directly against a CallGraph's
    # returned dicts) -- assert it holds, don't just trust it by design.
    def test_idPartCorrelation_samePartObtainedTwoWays_idsMatchCallGraphKeys(self):
        graph, parentPart, childPart = self.makeParentChildGraph()
        callGraph = graph.buildCallGraph()

        partViaSections = graph.sections["parent"].parts[0]
        partViaIter = next(iter(graph.iterByContentPart())).part

        self.assertEqual(id(parentPart), id(partViaSections))
        self.assertEqual(id(parentPart), id(partViaIter))
        self.assertIn(id(parentPart), callGraph.partsById)
        self.assertIs(callGraph.partsById[id(parentPart)], parentPart)
