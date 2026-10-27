import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


_Z3CTX = FRB.Z3Context()  # shared across every IfPredPart built in this test file


class GraphFilterTest(BaseUnitTest):
    # [root]
    # hash = h
    # if $x == 1
    #     ib = I
    #     drawindexed = auto
    # endif
    # run = CommandListChild
    def _makeGraph(self) -> FRB.IniSectionGraph:
        root = FRB.IfTemplate([
            FRB.IfContentPart({"hash": [(0, "h")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"ib": [(0, "I")], "drawindexed": [(1, "auto")]}, 1),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX),
            FRB.IfContentPart({"run": [(0, "CommandListChild")]}, 0)], name = "root")
        child = FRB.IfTemplate([FRB.IfContentPart({"a": [(0, "1")]}, 0)], name = "CommandListChild")
        return FRB.IniSectionGraph({"root": root, "CommandListChild": child}, ["root"], z3Ctx = _Z3CTX)

    def _keys(self, section: FRB.IfTemplate):
        result = []
        for part in section.parts:
            if (isinstance(part, FRB.IfContentPart)):
                result.append([key for key, _ in part.entries()])
            else:
                result.append(None)
        return result

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        graphIds = [(0, "comp", "a")]
        predicate = lambda section, part, ind: True
        edit = FRB.GraphFilter(graphIds, predicate)

        self.assertIs(edit.graphIds, graphIds)
        self.assertIs(edit.predicate, predicate)

    def test_init_predicateDefaultsToNone(self):
        self.assertIsNone(FRB.GraphFilter([(0, "comp", "a")]).predicate)

    # ================================================
    # ===================== edit ======================

    def test_edit_removesRejectedKVPs_keepsTheIfStructure(self):
        graph = self._makeGraph()
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): graph})]

        keep = {"ib", "drawindexed"}
        FRB.GraphFilter([(0, "comp", "a")], lambda section, part, ind: part.getByInd(ind)[0] in keep).edit(graphGroups, None)

        root = graphGroups[0].graphs[("comp", "a")].sections["root"]
        self.assertEqual(self._keys(root), [[], None, ["ib", "drawindexed"], None, []])

    def test_edit_predicateSeesTheSection(self):
        graph = self._makeGraph()
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): graph})]

        # keep only the child's KVPs
        FRB.GraphFilter([(0, "comp", "a")], lambda section, part, ind: section.name == "CommandListChild").edit(graphGroups, None)

        sections = graphGroups[0].graphs[("comp", "a")].sections
        self.assertEqual(self._keys(sections["root"]), [[], None, [], None, []])
        self.assertEqual(self._keys(sections["CommandListChild"]), [["a"]])

    def test_edit_decisionsAreMadeBeforeAnyRemoval(self):
        # a predicate that reads the KVP before the one it judges: removing as it went would shift
        # every index after the first removal, and the second drawindexed would be judged by the
        # wrong neighbour
        part = FRB.IfContentPart({"vb0": [(0, "X"), (2, "Y")], "drawindexed": [(1, "1, 0, 0"), (3, "2, 0, 0")]}, 0)
        graph = FRB.IniSectionGraph({"root": FRB.IfTemplate([part], name = "root")}, ["root"])
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): graph})]

        def notAfterX(section, part, ind):
            return not (part.getByInd(ind)[0] == "drawindexed" and part.getByInd(ind - 1) == ("vb0", "X"))

        FRB.GraphFilter([(0, "comp", "a")], notAfterX).edit(graphGroups, None)

        result = graphGroups[0].graphs[("comp", "a")].sections["root"].parts[0]
        self.assertEqual(result.entries(), [("vb0", "X"), ("vb0", "Y"), ("drawindexed", "2, 0, 0")])

    def test_edit_noPredicate_keepsEverything(self):
        graph = self._makeGraph()
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): graph})]

        FRB.GraphFilter([(0, "comp", "a")]).edit(graphGroups, None)

        root = graphGroups[0].graphs[("comp", "a")].sections["root"]
        self.assertEqual(self._keys(root), [["hash"], None, ["ib", "drawindexed"], None, ["run"]])

    def test_edit_onlyTheNamedGraphsAreFiltered(self):
        a, b = self._makeGraph(), self._makeGraph()
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): a, ("comp", "b"): b})]

        FRB.GraphFilter([(0, "comp", "a")], lambda section, part, ind: False).edit(graphGroups, None)

        self.assertEqual(self._keys(graphGroups[0].graphs[("comp", "b")].sections["root"]),
                         [["hash"], None, ["ib", "drawindexed"], None, ["run"]])

    def test_edit_missingIds_skippedSilently(self):
        graph = self._makeGraph()
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): graph})]

        FRB.GraphFilter([(5, "comp", "a"), (0, "comp", "missing")], lambda section, part, ind: False).edit(graphGroups, None)

        self.assertEqual(self._keys(graphGroups[0].graphs[("comp", "a")].sections["root"]),
                         [["hash"], None, ["ib", "drawindexed"], None, ["run"]])

    def test_edit_returnsTheSameGraphGroupsList(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph()})]
        self.assertIs(FRB.GraphFilter([(0, "comp", "a")]).edit(graphGroups, None), graphGroups)

    def test_edit_mutatedIdsList_isHonoured(self):
        graph = self._makeGraph()
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): graph})]
        graphIds = []
        edit = FRB.GraphFilter(graphIds, lambda section, part, ind: False)

        graphIds.append((0, "comp", "a"))
        edit.edit(graphGroups, None)

        self.assertEqual(self._keys(graphGroups[0].graphs[("comp", "a")].sections["root"]), [[], None, [], None, []])
