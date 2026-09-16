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
import re
import sys
##### EndExtImports

##### LocalImports
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB
##### EndLocalImports


def isTextureRegister(key: str) -> bool:
    return re.fullmatch(r"ps-t\d+", key) is not None


class RegRestrictTest(BaseUnitTest):
    # ================================================
    # =================== __init__ ====================

    def test_init_setsAttributes(self):
        allowedKeys = ["ps-t0", "ps-t1"]
        edit = FRB.RegRestrict(allowedKeys, isTextureRegister, False)

        self.assertIs(edit.allowedKeys, allowedKeys)
        self.assertIs(edit.keyFilter, isTextureRegister)
        self.assertFalse(edit.keepFirstOnly)

    def test_init_defaults(self):
        edit = FRB.RegRestrict()

        self.assertIsNone(edit.allowedKeys)
        self.assertIsNone(edit.keyFilter)
        self.assertTrue(edit.keepFirstOnly)

    def test_isSubclassOfBaseRegEdit(self):
        self.assertTrue(issubclass(FRB.RegRestrict, FRB.BaseRegEdit))

    # ================================================
    # ===================== edit ======================

    def test_edit_returnsTheSamePartInstance(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "A")]}, 0)
        self.assertIs(FRB.RegRestrict(["ps-t0"]).edit(part, "root", None), part)

    def test_edit_theMotivatingExample(self):
        # a Bennett body on BennettAdventure's body slot: the metal map lands on the register the
        # shifted light map already holds, and the shadow ramp on one the slot does not read
        part = FRB.IfContentPart({"ps-t1": [(0, "Diffuse")], "ps-t0": [(1, "NormalMap")],
                                  "ps-t2": [(2, "LightMap"), (3, "MetalMap")], "ps-t3": [(4, "ShadowRamp")],
                                  "ib": [(5, "Ib")]}, 0)

        FRB.RegRestrict(["ps-t0", "ps-t1", "ps-t2"], isTextureRegister).edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t1", "Diffuse"), ("ps-t0", "NormalMap"), ("ps-t2", "LightMap"), ("ib", "Ib")])

    def test_edit_ungovernedKey_leftAloneEvenWhenRepeatedOrNotAllowed(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "A")], "run": [(1, "X"), (2, "Y")]}, 0)

        FRB.RegRestrict(["ps-t0"], isTextureRegister).edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t0", "A"), ("run", "X"), ("run", "Y")])

    def test_edit_repeatedKey_keepsTheFirstBinding(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "first"), (2, "second"), (3, "third")], "ps-t1": [(1, "B")]}, 0)

        FRB.RegRestrict(None, isTextureRegister).edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t0", "first"), ("ps-t1", "B")])

    def test_edit_keepFirstOnlyFalse_keepsEveryAllowedBinding(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "first"), (1, "second")], "ps-t5": [(2, "C")]}, 0)

        FRB.RegRestrict(["ps-t0"], isTextureRegister, keepFirstOnly = False).edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t0", "first"), ("ps-t0", "second")])

    def test_edit_allowedKeysNone_onlyRemovesRepeats(self):
        part = FRB.IfContentPart({"ps-t7": [(0, "A"), (1, "B")], "ps-t9": [(2, "C")]}, 0)

        FRB.RegRestrict(None, isTextureRegister).edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t7", "A"), ("ps-t9", "C")])

    def test_edit_keyFilterNone_governsEveryKey(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "A")], "ib": [(1, "Ib")]}, 0)

        FRB.RegRestrict(["ps-t0"]).edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t0", "A")])

    def test_edit_emptyAllowedKeys_removesEveryGovernedKey(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "A")], "ib": [(1, "Ib")]}, 0)

        FRB.RegRestrict([], isTextureRegister).edit(part, "root", None)

        self.compareList(part.entries(), [("ib", "Ib")])

    def test_edit_partRanges_onlyTheRestrictedOccurencesAreConsidered(self):
        # the binding at index 0 is outside the ranges, so the first binding IN range is the one kept
        part = FRB.IfContentPart({"ps-t0": [(0, "outside"), (1, "first"), (2, "second")]}, 0)

        FRB.RegRestrict(None, isTextureRegister).edit(part, "root", None, partRanges = FRB.Ranges([(1, 3)]))

        self.compareList(part.entries(), [("ps-t0", "outside"), ("ps-t0", "first")])

    def test_edit_reassignedAttributes_takeEffect(self):
        part = FRB.IfContentPart({"ps-t0": [(0, "A")], "ps-t1": [(1, "B")]}, 0)
        edit = FRB.RegRestrict(["ps-t0", "ps-t1"], isTextureRegister)

        edit.allowedKeys = ["ps-t1"]
        edit.keyFilter = lambda key: key.startswith("ps-t")
        edit.edit(part, "root", None)

        self.compareList(part.entries(), [("ps-t1", "B")])
