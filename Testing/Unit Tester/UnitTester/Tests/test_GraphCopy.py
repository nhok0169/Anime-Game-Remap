import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


_Z3CTX = FRB.Z3Context()  # shared across every IfPredPart built in this test file


class GraphCopyTest(BaseUnitTest):
    # [root]
    # hash = h
    # if $x == 1
    #     ib = I
    #     drawindexed = auto
    # endif
    def _makeGraph(self) -> FRB.IniSectionGraph:
        root = FRB.IfTemplate([
            FRB.IfContentPart({"hash": [(0, "h")]}, 0),
            FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
            FRB.IfContentPart({"ib": [(0, "I")], "drawindexed": [(1, "auto")]}, 1),
            FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX)], name = "root")
        return FRB.IniSectionGraph({"root": root}, ["root"], z3Ctx = _Z3CTX)

    def _keys(self, section: FRB.IfTemplate):
        return [[key for key, _ in part.entries()] if isinstance(part, FRB.IfContentPart) else None for part in section.parts]

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        srcId, dstId = (0, "comp", "a"), (0, "comp", "b")
        predicate = lambda section, part, ind: True
        edit = FRB.GraphCopy(srcId, dstId, predicate, minimal = False, newPartIds = False)

        self.assertIs(edit.srcId, srcId)
        self.assertIs(edit.dstId, dstId)
        self.assertIs(edit.predicate, predicate)
        self.assertFalse(edit.minimal)
        self.assertFalse(edit.newPartIds)

    def test_init_defaults(self):
        edit = FRB.GraphCopy((0, "comp", "a"), (0, "comp", "b"))
        self.assertIsNone(edit.predicate)
        self.assertTrue(edit.minimal)
        self.assertTrue(edit.newPartIds)

    # ================================================
    # ===================== edit ======================

    def test_edit_noPredicate_copiesTheWholeGraph(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph()})]

        FRB.GraphCopy((0, "comp", "a"), (0, "comp", "b")).edit(graphGroups, None)

        copy = graphGroups[0].graphs[("comp", "b")]
        self.assertEqual(self._keys(copy.sections["root"]), [["hash"], None, ["ib", "drawindexed"], None])

    def test_edit_predicate_filtersOnlyTheCopy(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph()})]

        FRB.GraphCopy((0, "comp", "a"), (0, "comp", "b"), lambda section, part, ind: part.getByInd(ind)[0] != "hash").edit(graphGroups, None)

        self.assertEqual(self._keys(graphGroups[0].graphs[("comp", "b")].sections["root"]), [[], None, ["ib", "drawindexed"], None])
        self.assertEqual(self._keys(graphGroups[0].graphs[("comp", "a")].sections["root"]), [["hash"], None, ["ib", "drawindexed"], None])

    def test_edit_theCopyIsDeep(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph()})]
        FRB.GraphCopy((0, "comp", "a"), (0, "comp", "b")).edit(graphGroups, None)

        graphGroups[0].graphs[("comp", "b")].sections["root"].parts[0].addKVP("extra", "1")

        self.assertEqual(self._keys(graphGroups[0].graphs[("comp", "a")].sections["root"])[0], ["hash"])
        self.assertIsNot(graphGroups[0].graphs[("comp", "a")], graphGroups[0].graphs[("comp", "b")])

    def test_edit_dstInANewGroup_appendsExactlyOneGroup(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph()})]

        FRB.GraphCopy((0, "comp", "a"), (1, "comp", "b")).edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 2)
        self.assertIn(("comp", "b"), graphGroups[1].graphs)

    def test_edit_dstInAnExistingGroup_doesNotAppend(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph()})]

        FRB.GraphCopy((0, "comp", "a"), (0, "comp", "b")).edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 1)

    def test_edit_dstPastTheEnd_skippedSilently(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph()})]

        FRB.GraphCopy((0, "comp", "a"), (5, "comp", "b")).edit(graphGroups, None)

        self.assertEqual(len(graphGroups), 1)
        self.assertNotIn(("comp", "b"), graphGroups[0].graphs)

    def test_edit_missingSource_copiesNothing(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph()})]

        FRB.GraphCopy((0, "comp", "missing"), (0, "comp", "b")).edit(graphGroups, None)
        FRB.GraphCopy((3, "comp", "a"), (0, "comp", "b")).edit(graphGroups, None)

        self.assertNotIn(("comp", "b"), graphGroups[0].graphs)

    def test_edit_existingDst_isReplaced(self):
        other = FRB.IniSectionGraph({"other": FRB.IfTemplate([FRB.IfContentPart({"z": [(0, "9")]}, 0)], name = "other")}, ["other"])
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph(), ("comp", "b"): other})]

        FRB.GraphCopy((0, "comp", "a"), (0, "comp", "b")).edit(graphGroups, None)

        self.assertIn("root", graphGroups[0].graphs[("comp", "b")].sections)
        self.assertNotIn("other", graphGroups[0].graphs[("comp", "b")].sections)

    def test_edit_returnsTheSameGraphGroupsList(self):
        graphGroups = [FRB.IniGraphGroup({("comp", "a"): self._makeGraph()})]
        self.assertIs(FRB.GraphCopy((0, "comp", "a"), (0, "comp", "b")).edit(graphGroups, None), graphGroups)
