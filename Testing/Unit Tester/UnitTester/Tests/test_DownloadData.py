import os
import sys
import shutil
import tempfile
import unittest

from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class DownloadDataTest(unittest.TestCase):
    """:class:`DownloadData`'s data and the two things it builds -- a parser's use of it is in ``test_GIMIParser``"""

    def test_construct_keepsWhatItIsGiven(self):
        data = FRB.DownloadData("Blend", FRB.FileDownload("someURL", "AmberBlendRemapDL.buf"), refToSection = True,
                                downloadRefKVPs = [("handling", "skip")], resourceKVPs = {"type": "Buffer", "stride": "32"})

        self.assertEqual(data.name, "Blend")
        self.assertTrue(data.refToSection)
        self.assertEqual(data.download.url, "someURL")
        self.assertEqual(data.download.filename, "AmberBlendRemapDL.buf")
        self.assertEqual(data.downloadRefKVPs, [("handling", "skip")])
        self.assertEqual(data.resourceKVPs, [("type", "Buffer"), ("stride", "32")])

    def test_construct_defaults(self):
        data = FRB.DownloadData("Diffuse", FRB.FileDownload("someURL", "Diffuse.dds"))

        self.assertFalse(data.refToSection)
        self.assertEqual(data.downloadRefKVPs, [])
        self.assertEqual(data.resourceKVPs, [])

    def test_blendRefKVPs_skipThenDrawTheVertices(self):
        self.assertEqual(FRB.DownloadData.blendRefKVPs(21916), [("handling", "skip"), ("draw", "21916,0")])

    def test_addToPart_theReferenceThenTheRefKVPs(self):
        data = FRB.DownloadData("Blend", FRB.FileDownload("someURL", "b.buf"), downloadRefKVPs = FRB.DownloadData.blendRefKVPs(5))
        part = FRB.IfContentPart({}, 0)
        data.addToPart(part, "vb1", "ResourceBlend")

        self.assertEqual(part.getVals("vb1"), ["ResourceBlend"])
        self.assertEqual(part.getVals("handling"), ["skip"])
        self.assertEqual(part.getVals("draw"), ["5,0"])

    def test_createResSection_resourceKVPsThenTheFile(self):
        data = FRB.DownloadData("Ib", FRB.FileDownload("someURL", "HeadRemapDL.ib"), resourceKVPs = [("type", "Buffer")])
        section = data.createResSection("ResourceHeadIb")

        self.assertIsInstance(section, FRB.IfTemplate)
        self.assertEqual(section.name, "ResourceHeadIb")
        part = section.parts[0]
        self.assertEqual(part.getVals("type"), ["Buffer"])
        self.assertEqual(part.getVals("filename"), ["HeadRemapDL.ib"])

    def test_createFileDownload_aFreshDownloadEachTime(self):
        data = FRB.DownloadData("Diffuse", FRB.FileDownload("someURL", "Diffuse.dds"))
        first = data.createFileDownload("someFolder")
        second = data.createFileDownload("someFolder")

        self.assertIsInstance(first, FRB.RemapIniDownload)
        self.assertIsNot(first, second)
        # the download's own FileDownload is still usable afterwards
        self.assertEqual(data.download.filename, "Diffuse.dds")

    def test_aSubclassOverride_isReachedThroughTheInterface(self):
        class Marked(FRB.DownloadData):
            def addToPart(self, part, key, val):
                part.addKVP(key, val)
                part.addKVP("marked", "1")

        data = Marked("n", FRB.FileDownload("someURL", "f.dds"))
        part = FRB.IfContentPart({}, 0)
        data.addToPart(part, "ps-t0", "ResourceT")

        self.assertEqual(part.getVals("marked"), ["1"])

    def test_theInterface_unimplementedMethod_raises(self):
        class Partial(FRB.IniParseDownloadData):
            pass

        with self.assertRaises(Exception):
            Partial().createResSection("x")


class LoadedInisTest(unittest.TestCase):
    """:meth:`IniFile.loadedInis`: which .ini files of a folder the game loads"""

    def setUp(self):
        self._folder = tempfile.mkdtemp(prefix = "AGRemapLoadedInis")
        for name in ["mod.ini", "Textures.INI", "DISABLED_old.ini", "modRemapFix1.ini", "MyRemapFixes.ini", "notes.txt"]:
            with open(os.path.join(self._folder, name), "w") as f:
                f.write("")

    def tearDown(self):
        shutil.rmtree(self._folder, ignore_errors = True)

    def test_loadedInis_leavesOutDisabledAndGeneratedCopies(self):
        names = [os.path.basename(path) for path in FRB.IniFile.loadedInis(self._folder)]
        self.assertEqual(names, sorted(["mod.ini", "Textures.INI", "MyRemapFixes.ini"]))

    def test_loadedInis_exclude(self):
        names = [os.path.basename(path) for path in FRB.IniFile.loadedInis(self._folder, os.path.join(self._folder, "mod.ini"))]
        self.assertNotIn("mod.ini", names)
        self.assertIn("MyRemapFixes.ini", names)

    def test_loadedSiblings_theOthersOfItsFolder(self):
        ini = FRB.IniFile(os.path.join(self._folder, "mod.ini"))
        names = [os.path.basename(path) for path in ini.loadedSiblings()]
        self.assertEqual(names, sorted(["Textures.INI", "MyRemapFixes.ini"]))

    def test_loadedSiblings_noPath_isEmpty(self):
        self.assertEqual(FRB.IniFile(txt = "[A]\nx = 1\n").loadedSiblings(), [])
