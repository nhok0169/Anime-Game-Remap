import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RegRemoveTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        removeKeys = {"a": None}
        edit = FRB.RegRemove(removeKeys)
        self.assertIs(edit.removeKeys, removeKeys)

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSamePartInstance(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegRemove({"a": None})

        result = edit.edit(part, "root", None)
        self.assertIs(result, part)

    def test_edit_predicateNone_removesEveryOccurenceUnconditionally(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "a2": [(2, "3")]}, 0)
        edit = FRB.RegRemove({"a": None})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("b", "2"), ("a2", "3")])

    def test_edit_predicateGiven_onlyRemovesAcceptedOccurences(self):
        part = FRB.IfContentPart({"a": [(0, "no"), (1, "yes")]}, 0)
        edit = FRB.RegRemove({"a": lambda ind, val: val == "yes"})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "no")])

    def test_edit_multipleKeys_eachRemovedIndependently(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)
        edit = FRB.RegRemove({"a": None, "c": None})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("b", "2")])

    def test_edit_unmentionedKey_leftUnchanged(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "z": [(1, "9")]}, 0)
        edit = FRB.RegRemove({"a": None})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("z", "9")])

    def test_edit_partRanges_onlyRestrictedOccurencesEligibleForRemoval(self):
        # an occurrence outside 'partRanges' is not eligible for removal, even though its key is
        # unconditionally mentioned in 'removeKeys'
        part = FRB.IfContentPart({"a": [(0, "1"), (1, "2"), (2, "3")]}, 0)
        edit = FRB.RegRemove({"a": None})

        edit.edit(part, "root", None, partRanges = FRB.Ranges([(1, 2)]))
        self.compareList(part.entries(), [("a", "1"), ("a", "3")])
