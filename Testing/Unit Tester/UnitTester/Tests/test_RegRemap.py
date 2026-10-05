import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RegRemapTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        keyRemap = {"a": ["x"]}
        edit = FRB.RegRemap(keyRemap)
        self.assertIs(edit.keyRemap, keyRemap)

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSamePartInstance(self):
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        edit = FRB.RegRemap({"a": ["x"]})

        result = edit.edit(part, "root", None)
        self.assertIs(result, part)

    def test_edit_bareListSingleKey_renamesKey(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")], "c": [(2, "3")]}, 0)
        edit = FRB.RegRemap({"a": ["x"]})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("x", "1"), ("b", "2"), ("c", "3")])

    def test_edit_twoWaySwap_exchangesBothKeysInOnePass(self):
        # THE FACE DIFFUSE FIX RESTS ON THIS. Both directions in ONE RegRemap is a true swap,
        # because remapKeys rebuilds the part in a single pass and consults the rules once per
        # ORIGINAL key -- the ps-t0 -> ps-t1 result is never re-read as an input to ps-t1 -> ps-t0.
        # Two edits applied in sequence would collapse both registers onto one.
        part = FRB.IfContentPart({"ps-t0": [(0, "Diffuse")], "ps-t1": [(1, "LightMap")]}, 0)
        edit = FRB.RegRemap({"ps-t0": ["ps-t1"], "ps-t1": ["ps-t0"]})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("ps-t1", "Diffuse"), ("ps-t0", "LightMap")])

    def test_edit_twoWaySwap_onlyOneSidePresent_movesItAndAddsNothing(self):
        # the common case for a face section: it binds ps-t0 and says nothing about ps-t1, so the
        # other half of the swap finds nothing to move and must not invent a register
        part = FRB.IfContentPart({"hash": [(0, "1d064079")], "ps-t0": [(1, "Diffuse")]}, 0)
        edit = FRB.RegRemap({"ps-t0": ["ps-t1"], "ps-t1": ["ps-t0"]})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("hash", "1d064079"), ("ps-t1", "Diffuse")])

    def test_edit_bareListFanOut_producesOneEntryPerRule(self):
        # a bare list with more than one entry fans a single occurrence out into one new entry per rule
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)
        edit = FRB.RegRemap({"a": ["x", "y"]})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("x", "1"), ("y", "1"), ("b", "2")])

    def test_edit_bareListEmptyRules_removesOccurrence(self):
        # zero rules firing (an empty list) drops the occurrence entirely -- the bare-list default,
        # as opposed to KeyRemapData(keepKeyWithoutRemap=True)
        part = FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0)
        edit = FRB.RegRemap({"a": []})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("b", "2")])

    def test_edit_remappedKeyDataPredicateRejects_occurrenceRemoved(self):
        part = FRB.IfContentPart({"a": [(0, "no"), (1, "yes")]}, 0)
        edit = FRB.RegRemap({"a": [FRB.RemappedKeyData("x", check = lambda k, v: v == "yes")]})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("x", "yes")])

    def test_edit_keyRemapDataKeepKeyWithoutRemap_retainsOriginalWhenNoRuleFires(self):
        part = FRB.IfContentPart({"a": [(0, "no"), (1, "yes")]}, 0)
        edit = FRB.RegRemap({"a": FRB.KeyRemapData([FRB.RemappedKeyData("x", check = lambda k, v: v == "yes")], keepKeyWithoutRemap = True)})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("a", "no"), ("x", "yes")])

    def test_edit_unmentionedKey_leftUnchanged(self):
        part = FRB.IfContentPart({"a": [(0, "1")], "z": [(1, "9")]}, 0)
        edit = FRB.RegRemap({"a": ["x"]})

        edit.edit(part, "root", None)
        self.compareList(part.entries(), [("x", "1"), ("z", "9")])

    def test_edit_partRanges_onlyRestrictedOccurrencesRemapped(self):
        # an occurrence outside 'partRanges' is treated as a pure pass-through, exactly as if its key
        # were never mentioned in 'keyRemap' at all
        part = FRB.IfContentPart({"a": [(0, "1"), (1, "2"), (2, "3")]}, 0)
        edit = FRB.RegRemap({"a": ["x"]})

        edit.edit(part, "root", None, partRanges = FRB.Ranges([(1, 2)]))
        self.compareList(part.entries(), [("a", "1"), ("x", "2"), ("a", "3")])
