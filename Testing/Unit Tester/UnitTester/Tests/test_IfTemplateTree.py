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


# NOTE: only one tree class ("IfTemplateTree") is bound for Python -- a tree is only ever reached
# via IfTemplate.tree (built with the "non-empty-node" tree kind by IfTemplate's own constructor,
# or with the "norm" tree kind after calling IfTemplate.normalize()), never constructed directly.
class IfTemplateTreeTest(BaseUnitTest):
    # ========= construct (default, "non-empty-node" tree kind) ====================

    def test_noBranching_flatTreeWithNoChildren(self):
        parts = [FRB.IfContentPart({"S3FIFOVariant": [(0, "4")], "cacheType": []}, 0)]
        tree = FRB.IfTemplate(parts).tree

        # (ifPredPart is None, parts = [<content part>], children = [])
        expected = (None, [0], [])
        self.compareIfTemplateTree(tree.root, expected, parts)

    def test_simpleIfElse_twoChildrenBothLeaves(self):
        parts = [FRB.IfContentPart({"InductionType": [(0, "Strong Induction (POSI)")], "inductiveHypothesis": [(1, "false")]}, 0),
                 FRB.IfPredPart("if $i == 0 then", FRB.IfPredPartType.If, _Z3CTX),
                 FRB.IfContentPart({"baseCase": [(0, "0")]}, 1),
                 FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                 FRB.IfContentPart({"inductiveHypothesis": [(0, "true")], "inductiveStep": [(1, "true")]}, 1),
                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]
        tree = FRB.IfTemplate(parts).tree

        expected = (None, [0, None, None], [(1, [2], []), (3, [4], [])])
        self.compareIfTemplateTree(tree.root, expected, parts)

    def test_nestedAndElifBranches_multiLevelTree(self):
        parts = [FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 0),
                 FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                     FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                         FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                     FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                 FRB.IfPredPart("else if $x % 5 == 0", FRB.IfPredPartType.Elif, _Z3CTX),
                     FRB.IfPredPart("if $x % 6 == 0", FRB.IfPredPartType.If, _Z3CTX),
                     FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 2),
                     FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                 FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 1),
                     FRB.IfPredPart("if $x % 7 == 0", FRB.IfPredPartType.If, _Z3CTX),
                     FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                 FRB.IfPredPart("else if $x % 10 == 0", FRB.IfPredPartType.Elif, _Z3CTX),
                     FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                     FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                     FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                     FRB.IfPredPart("if $x % 3 == 0", FRB.IfPredPartType.If, _Z3CTX),
                     FRB.IfContentPart({"a": [(0, "1")], "b": [(1, "2")]}, 2),
                     FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX),
                     FRB.IfContentPart({"target": [(0, "1")], "b": [(1, "2")]}, 1),
                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]
        tree = FRB.IfTemplate(parts).tree

        # NOTE: the "if $x % 7 == 0" branch has nothing between it and its "endIf" (no content, no
        # nested if) -- the tree builder gives that node one synthetic *empty* IfContentPart rather
        # than zero parts. This is specific to the "non-empty-node" tree kind (the one IfTemplate's
        # constructor uses by default, and the one under test here) -- the plain base tree kind
        # (used only for TreeKind::Basic / IfTemplate.add) does not synthesize a placeholder.
        expected = (None, [0, None, None, None], [(1, [None], [(2, [3], [])]),
                                                    (5, [None, 9, None], [(6, [7], []), (10, ["<empty>"], [])]),
                                                    (12, [None, None, 19], [(13, [14], []), (16, [17], [])])])
        self.compareIfTemplateTree(tree.root, expected, parts)

    # ========================================================
    # ========= normalize() rebuilds using the "norm" tree kind, inserting a synthetic empty
    # ========= "else" branch for every "if" (or "elif" chain) that doesn't already end with one
    # ========================================================

    def test_normalize_ifWithNoElse_syntheticEmptyElseInserted(self):
        parts = [FRB.IfContentPart({"a": [(0, "1")]}, 0),
                 FRB.IfPredPart("if $i == 0", FRB.IfPredPartType.If, _Z3CTX),
                 FRB.IfContentPart({"b": [(0, "2")]}, 1),
                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]
        ifTemplate = FRB.IfTemplate(parts)

        ifTemplate.normalize()

        # a synthetic "else" IfPredPart plus a new, empty IfContentPart were inserted right before
        # "endIf"
        self.assertEqual(len(ifTemplate), 6)
        self.assertIsInstance(ifTemplate[3], FRB.IfPredPart)
        self.assertEqual(ifTemplate[3].type, FRB.IfPredPartType.Else)
        self.assertIsInstance(ifTemplate[4], FRB.IfContentPart)
        self.assertEqual(ifTemplate[4].entries(), [])
        self.assertIsInstance(ifTemplate[5], FRB.IfPredPart)
        self.assertEqual(ifTemplate[5].type, FRB.IfPredPartType.EndIf)

        tree = ifTemplate.tree
        expected = (None, [0, None, None], [(1, [2], []), (3, [4], [])])
        self.compareIfTemplateTree(tree.root, expected, ifTemplate.parts)

    def test_normalize_ifWithExistingElse_noNewPartsInserted(self):
        parts = [FRB.IfContentPart({"a": [(0, "1")]}, 0),
                 FRB.IfPredPart("if $i == 0", FRB.IfPredPartType.If, _Z3CTX),
                 FRB.IfContentPart({"b": [(0, "2")]}, 1),
                 FRB.IfPredPart("else", FRB.IfPredPartType.Else, _Z3CTX),
                 FRB.IfContentPart({"c": [(0, "3")]}, 1),
                 FRB.IfPredPart("endIf", FRB.IfPredPartType.EndIf, _Z3CTX)]
        ifTemplate = FRB.IfTemplate(parts)

        ifTemplate.normalize()
        self.assertEqual(len(ifTemplate), 6)

    # ========================================================
    # ========= clear() ========================================

    def test_clear_treeBecomesEmpty(self):
        parts = [FRB.IfContentPart({"a": [(0, "1")]}, 0)]
        tree = FRB.IfTemplate(parts).tree

        self.assertIsNotNone(tree.root)
        tree.clear()
        self.assertIsNone(tree.root)
