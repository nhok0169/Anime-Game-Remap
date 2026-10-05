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


class IfTemplateNodeTest(BaseUnitTest):
    # ========== construct ====================================

    def test_construct_noId_idIsGenerated(self):
        node = FRB.IfTemplateNode()
        self.assertIsNotNone(node.id)
        self.assertIsInstance(node.id, int)

    def test_construct_explicitId_idIsGiven(self):
        node = FRB.IfTemplateNode(id = 5)
        self.assertEqual(node.id, 5)

    def test_construct_twoNodesNoId_idsAreDistinct(self):
        node1 = FRB.IfTemplateNode()
        node2 = FRB.IfTemplateNode()
        self.assertNotEqual(node1.id, node2.id)

    def test_construct_noIfPredPart_ifPredPartIsNone(self):
        node = FRB.IfTemplateNode()
        self.assertIsNone(node.ifPredPart)

    def test_construct_withIfPredPart_ifPredPartIsStored(self):
        pred = FRB.IfPredPart("some condition", FRB.IfPredPartType.If, _Z3CTX)
        node = FRB.IfTemplateNode(ifPredPart = pred)
        self.assertIs(node.ifPredPart, pred)

    def test_construct_noArgs_childrenAndPartsAreEmpty(self):
        node = FRB.IfTemplateNode()
        self.assertEqual(node.children, {})
        self.assertEqual(node.parts, [])

    # ========== addChild ====================================

    def test_addChild_singleChild_childInChildrenAndParts(self):
        parent = FRB.IfTemplateNode()
        child = FRB.IfTemplateNode()
        parent.addChild(child)

        self.assertEqual(len(parent.children), 1)
        self.assertIs(parent.children[child.id], child)
        self.assertEqual(len(parent.parts), 1)
        self.assertIs(parent.parts[0], child)

    def test_addChild_multipleChildren_allChildrenAdded(self):
        parent = FRB.IfTemplateNode()
        child1 = FRB.IfTemplateNode()
        child2 = FRB.IfTemplateNode()
        parent.addChild(child1)
        parent.addChild(child2)

        self.assertEqual(len(parent.children), 2)
        self.assertIs(parent.children[child1.id], child1)
        self.assertIs(parent.children[child2.id], child2)
        self.assertEqual(len(parent.parts), 2)

    # ========== addIfContentPart ====================================

    def test_addIfContentPart_singlePart_partInParts(self):
        node = FRB.IfTemplateNode()
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        node.addIfContentPart(part)

        self.assertEqual(len(node.parts), 1)
        self.assertIs(node.parts[0], part)
        # adding a content part must not register it as a "child" node
        self.assertEqual(node.children, {})

    def test_addIfContentPart_mixedWithChild_orderIsPreserved(self):
        node = FRB.IfTemplateNode()
        part1 = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        child = FRB.IfTemplateNode()
        part2 = FRB.IfContentPart({"b": [(0, "2")]}, 0)

        node.addIfContentPart(part1)
        node.addChild(child)
        node.addIfContentPart(part2)

        self.assertEqual(len(node.parts), 3)
        self.assertIs(node.parts[0], part1)
        self.assertIs(node.parts[1], child)
        self.assertIs(node.parts[2], part2)

    # ========== hasKey ====================================

    def test_hasKey_keyInSomePart_true(self):
        node = FRB.IfTemplateNode()
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "1")]}, 0))
        node.addIfContentPart(FRB.IfContentPart({"b": [(0, "2")]}, 0))
        self.assertTrue(node.hasKey("b"))

    def test_hasKey_keyMissingFromEveryPart_false(self):
        node = FRB.IfTemplateNode()
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "1")]}, 0))
        self.assertFalse(node.hasKey("c"))

    def test_hasKey_noParts_false(self):
        node = FRB.IfTemplateNode()
        self.assertFalse(node.hasKey("a"))

    # ========== getKeyPart ====================================

    def test_getKeyPart_keyInOnePart_returnsThatPart(self):
        node = FRB.IfTemplateNode()
        part = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        node.addIfContentPart(part)
        self.assertIs(node.getKeyPart("a"), part)

    def test_getKeyPart_keyInMultipleParts_returnsLatestPart(self):
        node = FRB.IfTemplateNode()
        part1 = FRB.IfContentPart({"a": [(0, "1")]}, 0)
        part2 = FRB.IfContentPart({"a": [(0, "2")]}, 0)
        node.addIfContentPart(part1)
        node.addIfContentPart(part2)
        self.assertIs(node.getKeyPart("a"), part2)

    def test_getKeyPart_keyMissing_returnsNone(self):
        node = FRB.IfTemplateNode()
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "1")]}, 0))
        self.assertIsNone(node.getKeyPart("z"))

    # ========== getKeyVal ====================================

    def test_getKeyVal_keyInOnePart_returnsItsLatestValue(self):
        node = FRB.IfTemplateNode()
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "1"), (1, "2")]}, 0))
        self.assertEqual(node.getKeyVal("a"), "2")

    def test_getKeyVal_keyInLaterPart_returnsThatPartsValue(self):
        node = FRB.IfTemplateNode()
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "1")]}, 0))
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "2")]}, 0))
        self.assertEqual(node.getKeyVal("a"), "2")

    def test_getKeyVal_keyMissing_returnsNone(self):
        node = FRB.IfTemplateNode()
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "1")]}, 0))
        self.assertIsNone(node.getKeyVal("z"))

    # ========== getKeyValues ====================================

    def test_getKeyValues_keyInMultipleParts_allOccurencesReturnedPerPart(self):
        node = FRB.IfTemplateNode()
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "1"), (1, "2")]}, 0))
        node.addIfContentPart(FRB.IfContentPart({"b": [(0, "x")]}, 0))
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "3")]}, 0))

        result = node.getKeyValues("a")
        self.assertEqual(result, [[(0, "1"), (1, "2")], [(0, "3")]])

    def test_getKeyValues_keyMissing_returnsEmptyList(self):
        node = FRB.IfTemplateNode()
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "1")]}, 0))
        self.assertEqual(node.getKeyValues("z"), [])

    # ========== getKeyMissingPart ====================================

    def test_getKeyMissingPart_keyFoundSomewhere_returnsNoneTrue(self):
        node = FRB.IfTemplateNode()
        node.addIfContentPart(FRB.IfContentPart({"b": [(0, "1")]}, 0))
        node.addIfContentPart(FRB.IfContentPart({"a": [(0, "1")]}, 0))
        result = node.getKeyMissingPart("a")
        self.assertEqual(result, (None, True))

    def test_getKeyMissingPart_keyNeverFound_returnsFirstPartTrue(self):
        node = FRB.IfTemplateNode()
        firstPart = FRB.IfContentPart({"b": [(0, "1")]}, 0)
        node.addIfContentPart(firstPart)
        node.addIfContentPart(FRB.IfContentPart({"c": [(0, "1")]}, 0))
        result = node.getKeyMissingPart("a")
        self.assertIs(result[0], firstPart)
        self.assertTrue(result[1])

    def test_getKeyMissingPart_noContentParts_returnsNoneFalse(self):
        node = FRB.IfTemplateNode()
        result = node.getKeyMissingPart("a")
        self.assertEqual(result, (None, False))
