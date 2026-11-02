import sys
import unittest

from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ListValueTest(unittest.TestCase):
    """:class:`ListValue`: a `KVP`'s value read as a list of fields, of any length"""

    # ====================== reading ====================================

    def test_oneField_aPlainValueIsAListOfOne(self):
        value = FRB.ListValue("ResourceX")
        self.assertEqual(value.size(), 1)
        self.assertEqual(value.at(0), "ResourceX")
        self.assertEqual(value.at(-1), "ResourceX")
        self.assertIsNone(value.number(0))

    def test_twoFields(self):
        value = FRB.ListValue("6, 36")
        self.assertEqual(value.fields, ["6", "36"])
        self.assertEqual(value.numbers(), [6, 36])

    def test_eightFields(self):
        value = FRB.ListValue("1,2, 3 ,4,5,6,7,8")
        self.assertEqual(value.size(), 8)
        self.assertEqual(value.number(2), 3)
        self.assertEqual(value.number(-1), 8)
        self.assertEqual(value.numbers(), [1, 2, 3, 4, 5, 6, 7, 8])

    def test_empty_hasNoFields(self):
        self.assertEqual(FRB.ListValue("").size(), 0)
        self.assertEqual(FRB.ListValue("   ").size(), 0)
        self.assertIsNone(FRB.ListValue("").numbers())

    def test_trailingSeparator_givesAnEmptyLastField(self):
        value = FRB.ListValue("a, b,")
        self.assertEqual(value.fields, ["a", "b", ""])
        self.assertIsNone(value.number(2))

    def test_pastEitherEnd_isNone(self):
        value = FRB.ListValue("a, b")
        self.assertIsNone(value.at(2))
        self.assertIsNone(value.at(-3))
        self.assertIsNone(value.number(5))

    def test_number_onlyAnOptionalMinusThenDigits(self):
        value = FRB.ListValue("-4, 07, auto, $s, 1.5, +2, -")
        self.assertEqual([value.number(i) for i in range(value.size())], [-4, 7, None, None, None, None, None])
        self.assertIsNone(value.numbers())

    def test_otherSeparator(self):
        value = FRB.ListValue("a b c", " ")
        self.assertEqual(value.fields, ["a", "b", "c"])

    # ====================== writing ====================================

    def test_untouched_isWrittenExactlyAsGiven(self):
        self.assertEqual(FRB.ListValue("  12,30 ,0").str(), "  12,30 ,0")
        self.assertFalse(FRB.ListValue("12, 30, 0").changed())

    def test_set_writesTheFieldsJoined(self):
        value = FRB.ListValue("12,30 ,0")
        self.assertTrue(value.setNumber(1, 60))
        self.assertTrue(value.changed())
        self.assertEqual(value.str(), "12, 60, 0")

    def test_set_pastTheEnd_changesNothing(self):
        value = FRB.ListValue("auto")
        self.assertFalse(value.setNumber(1, 30))
        self.assertFalse(value.changed())
        self.assertEqual(value.str(), "auto")

    def test_set_fromTheEnd(self):
        value = FRB.ListValue("a, b, c, d, e, f, g, h")
        value.set(-1, " z ")
        self.assertEqual(value.str(), "a, b, c, d, e, f, g, z")

    def test_append(self):
        value = FRB.ListValue("a")
        value.append("b")
        self.assertEqual(value.str(), "a, b")
        self.assertEqual(len(value), 2)


class IbFileRemapDrawRangeTest(unittest.TestCase):
    """:meth:`IbFile.remapDrawRange`: a draw range moved onto a copy of an index buffer that kept some triangles"""

    KEPT = [0, 1, 2, 5, 6, 7]   # triangles 3 and 4 removed

    def test_rangeBeforeTheRemoval_isUnchanged(self):
        self.assertEqual(FRB.IbFile.remapDrawRange(9, 0, self.KEPT), (9, 0))

    def test_rangeOverTheRemoval_keepsWhatIsLeft(self):
        self.assertEqual(FRB.IbFile.remapDrawRange(9, 9, self.KEPT), (3, 9))

    def test_rangeAfterTheRemoval_movesDown(self):
        self.assertEqual(FRB.IbFile.remapDrawRange(6, 18, self.KEPT), (6, 12))

    def test_rangeOfOnlyRemovedTriangles_isEmpty(self):
        self.assertEqual(FRB.IbFile.remapDrawRange(6, 9, self.KEPT), (0, 9))


class SectionTextTest(unittest.TestCase):
    """:class:`SectionText`: a section built from its lines"""

    def test_build_aSectionWithAnIfBlock(self):
        z3 = FRB.Z3Context()
        section = (FRB.SectionText(z3, "TextureOverrideBody")
                   .keys([("hash", "abcd1234"), ("match_first_index", "0")])
                   .open("$mod_enabled")
                   .key("handling", "skip")
                   .close()
                   .build())

        self.assertIsInstance(section, FRB.IfTemplate)
        self.assertEqual(section.name, "TextureOverrideBody")

    def test_str_rendersTheLinesInOrder(self):
        z3 = FRB.Z3Context()
        text = FRB.SectionText(z3, "TextureOverrideBody").prefix("; a note").key("hash", "abcd1234").open("$on").key("x", "1").close().str()

        self.assertIn("[TextureOverrideBody]", text)
        for earlier, later in [("; a note", "[TextureOverrideBody]"), ("hash = abcd1234", "if $on"), ("if $on", "x = 1"), ("x = 1", "endif")]:
            self.assertLess(text.index(earlier), text.index(later))
