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


_Z3CTX = FRB.Z3Context()  # shared across every IfPredPart built in this test file
Position = FRB.RegIfTemplateAdd.Position


# RegIfTemplateAdd is C++-backed: it inserts the parts of an IfTemplate -- if blocks included -- into
# every root, at the top, at the bottom, or right after a key.
class RegIfTemplateAddTest(BaseUnitTest):
    def _section(self):
        """hash, match_first_index, then a draw under `if $x == 1`"""
        return FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "h")], "match_first_index": [(1, "0")], "ib": [(2, "I")]}, 0),
                               FRB.IfPredPart("if $x == 1", FRB.IfPredPartType.If, _Z3CTX),
                               FRB.IfContentPart({"drawindexed": [(0, "5, 0, 0")]}, 1),
                               FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX)])

    def _template(self):
        """if ps == 1 / ps-cb3 = A / else / ps-cb3 = B / endif"""
        return FRB.IfTemplate([FRB.IfPredPart("if ps == 1", FRB.IfPredPartType.If, _Z3CTX),
                               FRB.IfContentPart({"ps-cb3": [(0, "A")]}, 1),
                               FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                               FRB.IfContentPart({"ps-cb3": [(0, "B")]}, 1),
                               FRB.IfPredPart("endif", FRB.IfPredPartType.EndIf, _Z3CTX)])

    def _shape(self, section: FRB.IfTemplate):
        """each part as its `if` line, or (depth, entries)"""
        return [(part.depth, part.entries()) if isinstance(part, FRB.IfContentPart) else part.src.strip() for part in section.parts]

    _BLOCK = ["if ps == 1", (1, [("ps-cb3", "A")]), "else", (1, [("ps-cb3", "B")]), "endif"]

    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        template = self._template()
        edit = FRB.RegIfTemplateAdd(template, Position.AfterKey, "match_first_index")
        self.assertIs(edit.ifTemplate, template)
        self.assertEqual(edit.position, Position.AfterKey)
        self.assertEqual(edit.key, "match_first_index")

    def test_init_defaults(self):
        edit = FRB.RegIfTemplateAdd()
        self.assertIsNone(edit.ifTemplate)
        self.assertEqual(edit.position, Position.Bottom)
        self.assertEqual(edit.key, "")

    def test_isSubclassOfBaseIniGraphEdit(self):
        self.assertTrue(issubclass(FRB.RegIfTemplateAdd, FRB.BaseIniGraphEdit))

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSameGraphObject(self):
        graph = FRB.IniSectionGraph({"root": self._section()}, ["root"], z3Ctx = _Z3CTX)
        self.assertIs(FRB.RegIfTemplateAdd(self._template()).edit(graph, None), graph)

    def test_edit_afterKey_cutsThePartAfterTheKey(self):
        section = self._section()
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegIfTemplateAdd(self._template(), Position.AfterKey, "match_first_index").edit(graph, None)

        self.compareList(self._shape(section), [(0, [("hash", "h"), ("match_first_index", "0")])] + self._BLOCK + [
            (0, [("ib", "I")]), "if $x == 1", (1, [("drawindexed", "5, 0, 0")]), "endif"])

    def test_edit_afterKey_lastKVPOfAPart_noEmptyTail(self):
        section = FRB.IfTemplate([FRB.IfContentPart({"hash": [(0, "h")], "match_first_index": [(1, "0")]}, 0)])
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegIfTemplateAdd(self._template(), Position.AfterKey, "match_first_index").edit(graph, None)

        self.compareList(self._shape(section), [(0, [("hash", "h"), ("match_first_index", "0")])] + self._BLOCK)

    def test_edit_afterKey_missingKey_leavesTheRootAlone(self):
        section = self._section()
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)
        before = self._shape(section)

        FRB.RegIfTemplateAdd(self._template(), Position.AfterKey, "nope").edit(graph, None)

        self.compareList(self._shape(section), before)

    def test_edit_top(self):
        section = self._section()
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegIfTemplateAdd(self._template(), Position.Top).edit(graph, None)

        self.compareList(self._shape(section)[:5], self._BLOCK)
        self.assertEqual(len(section.parts), 9)

    def test_edit_bottom_atTheSectionsOwnDepth(self):
        section = self._section()
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegIfTemplateAdd(self._template(), Position.Bottom).edit(graph, None)

        self.compareList(self._shape(section)[4:], self._BLOCK)

    def test_edit_insideABlock_shiftsTheTemplatesDepth(self):
        # the key sits inside `if $x == 1`, at depth 1: the template's depth-1 KVPs land at depth 2
        section = self._section()
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)

        FRB.RegIfTemplateAdd(self._template(), Position.AfterKey, "drawindexed").edit(graph, None)

        shape = self._shape(section)
        self.compareList(shape[3:8], ["if ps == 1", (2, [("ps-cb3", "A")]), "else", (2, [("ps-cb3", "B")]), "endif"])

    def test_edit_oneTemplate_servesEveryRoot(self):
        a, b = self._section(), self._section()
        graph = FRB.IniSectionGraph({"a": a, "b": b}, ["a", "b"], z3Ctx = _Z3CTX)
        template = self._template()

        FRB.RegIfTemplateAdd(template, Position.Bottom).edit(graph, None)

        self.compareList(self._shape(a)[4:], self._BLOCK)
        self.compareList(self._shape(b)[4:], self._BLOCK)
        self.assertEqual(len(template.parts), 5)  # the template itself is untouched

    def test_edit_noTemplate_isANoOp(self):
        section = self._section()
        graph = FRB.IniSectionGraph({"root": section}, ["root"], z3Ctx = _Z3CTX)
        before = self._shape(section)

        FRB.RegIfTemplateAdd(None).edit(graph, None)

        self.compareList(self._shape(section), before)
