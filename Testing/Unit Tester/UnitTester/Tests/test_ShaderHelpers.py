import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ShaderFilterTagsTest(BaseUnitTest):
    @classmethod
    def tags(cls) -> FRB.ShaderFilterTags:
        tags = FRB.ShaderFilterTags()
        tags.add("aaaa", "1.1")
        tags.add("bbbb", "1.2")
        return tags

    def test_add_theFirstValueStays_inTheOrderTagged(self):
        tags = self.tags()

        self.assertFalse(tags.add("aaaa", "9.9"))
        self.assertEqual([(tag.shaderHash, tag.filterIndex) for tag in tags.tags()], [("aaaa", "1.1"), ("bbbb", "1.2")])
        self.assertEqual(tags.filterIndexOf("aaaa"), "1.1")
        self.assertIsNone(tags.filterIndexOf("cccc"))

    def test_condition_any_eachShaderOnce_untaggedLeftOut(self):
        self.assertEqual(self.tags().condition("ps", ["bbbb", "cccc", "aaaa", "bbbb"]), "ps == 1.2 || ps == 1.1")

    def test_condition_twoShadersOneIndex_namedOnce(self):
        tags = self.tags()
        tags.add("dddd", "1.2")
        self.assertEqual(tags.condition("vs", ["bbbb", "dddd", "aaaa"]), "vs == 1.2 || vs == 1.1")
        self.assertEqual(tags.condition("vs", ["dddd", "bbbb"], none = True), "vs != 1.2")

    def test_condition_none(self):
        self.assertEqual(self.tags().condition("vs", ["aaaa", "bbbb"], none = True), "vs != 1.1 && vs != 1.2")

    def test_condition_noShaderTagged_empty(self):
        self.assertEqual(self.tags().condition("ps", ["cccc"]), "")

    def test_section_hashAndFilterIndex(self):
        section = self.tags().section(1, "ShaderOverrideB")

        self.assertEqual(section.name, "ShaderOverrideB")
        text = section.toStr()
        self.assertIn("hash = bbbb", text)
        self.assertIn("filter_index = 1.2", text)

    def test_section_noSuchShader_raises(self):
        with self.assertRaises(IndexError):
            self.tags().section(5, "ShaderOverrideMissing")


    def test_addGiven_theGivenValues_andRefusesAShaderWithNone(self):
        tags = FRB.ShaderFilterTags()
        tags.add("aa", "1.5")
        # "aa" is tagged already and keeps its value
        tags.addGiven(["aa", "bb", "cc", "bb"], {"aa": "7", "bb": "100", "cc": "9"})
        self.assertEqual([(tag.shaderHash, tag.filterIndex) for tag in tags.tags()], [("aa", "1.5"), ("bb", "100"), ("cc", "9")])
        self.assertRaises(ValueError, tags.addGiven, ["dd"], {"cc": "9"})


class CustomShaderBackupsTest(BaseUnitTest):
    @classmethod
    def backups(cls) -> FRB.CustomShaderBackups:
        return FRB.CustomShaderBackups(lambda reg: "ResourceBackup" + reg.replace("-", ""))

    def test_bindsRegister_resourceRegistersOnly(self):
        for key in ["ps-t4", "PS-CB12", " vs-t0 "]:
            self.assertTrue(FRB.CustomShaderBackups.bindsRegister(key), key)
        for key in ["ps", "ps-t", "ps-x4", "vb0", "drawindexed", "ps-t4a", "o0"]:
            self.assertFalse(FRB.CustomShaderBackups.bindsRegister(key), key)

    def test_wrap_savesBeforeAndRestoresAfter_registersSorted(self):
        backups = self.backups()
        result = backups.wrap([("run", "CustomShaderA")], [("ps-t4", "ResourceMask"), ("drawindexed", "auto"), ("ps-cb3", "ResourceCb")])

        self.assertEqual(result, [("ResourceBackuppscb3", "ref ps-cb3"), ("ResourceBackuppst4", "ref ps-t4"),
                                  ("run", "CustomShaderA"),
                                  ("ps-cb3", "ref ResourceBackuppscb3"), ("ps-t4", "ref ResourceBackuppst4")])

    def test_wrap_nothingAssigned_runsUnchanged(self):
        self.assertEqual(self.backups().wrap([("run", "CustomShaderA")], [("drawindexed", "auto")]), [("run", "CustomShaderA")])

    def test_registers_everyWrap_untilClear(self):
        backups = self.backups()
        backups.wrap([("run", "A")], [("ps-t4", "X")])
        backups.wrap([("run", "B")], [("ps-t1", "Y"), ("ps-t4", "Z")])

        self.assertEqual(backups.registers(), ["ps-t1", "ps-t4"])
        backups.clear()
        self.assertEqual(backups.registers(), [])

    def test_section_emptyAndNamed(self):
        section = self.backups().section("ps-t4")

        self.assertEqual(section.name, "ResourceBackuppst4")
        self.assertEqual(len(section), 0)
