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



class IniSharedStateTest(unittest.TestCase):
    """:class:`IniSharedState`: the files of a run and the links their text states -- a namespace one references
    that another declares, or a resource file both name -- wherever the files are"""

    def setUp(self):
        self._root = tempfile.mkdtemp(prefix = "AGRemapSharedState")
        self._write("mesh/mod.ini", "namespace = MyMod\n[TextureOverrideBody]\nhash = 11111111\nps-t0 = ResourceDiffuse\n"
                                    "[ResourceDiffuse]\nfilename = Diffuse.dds\n")
        self._write("elsewhere/far/textures.ini", "[TextureOverrideRecolour]\nhash = 22222222\nif $\\MyMod\\color == 1\n"
                                                  "    this = ResourceRed\nendif\n[ResourceRed]\nfilename = Red.dds\n")
        self._write("mesh/sub/copy.ini", "[ResourceDiffuse]\nfilename = ..\\Diffuse.dds\n")
        self._write("mesh/other.ini", "[TextureOverrideBody]\nhash = 11111111\nps-t0 = ResourceOwn\n"
                                      "[ResourceOwn]\nfilename = Own.dds\n")
        self._state = FRB.IniSharedState()
        for rel in ["mesh/mod.ini", "elsewhere/far/textures.ini", "mesh/sub/copy.ini", "mesh/other.ini"]:
            self._state.add(self._path(rel))

    def tearDown(self):
        shutil.rmtree(self._root, ignore_errors = True)

    def _path(self, rel):
        return os.path.join(self._root, *rel.split("/"))

    def _write(self, rel, text):
        path = self._path(rel)
        os.makedirs(os.path.dirname(path), exist_ok = True)
        with open(path, "w") as f:
            f.write(text)

    def _linked(self, rel):
        return sorted(os.path.normcase(path) for path in self._state.linked(self._path(rel)))

    def _paths(self, *rels):
        return sorted(os.path.normcase(self._path(rel)) for rel in rels)

    def test_namespace_isTheDeclaredOne(self):
        self.assertEqual(self._state.namespaceOf(self._path("mesh/mod.ini")), "MyMod")
        self.assertIsNone(self._state.namespaceOf(self._path("mesh/other.ini")))

    def test_linked_byANamespaceReference_inAnyFolder(self):
        self.assertIn(self._paths("elsewhere/far/textures.ini")[0], self._linked("mesh/mod.ini"))
        self.assertEqual(self._linked("elsewhere/far/textures.ini"), self._paths("mesh/mod.ini"))

    def test_linked_byAResourceFileBothName(self):
        self.assertIn(self._paths("mesh/sub/copy.ini")[0], self._linked("mesh/mod.ini"))

    def test_notLinked_byFolderOrHashAlone(self):
        # beside it, and on the same hash, but naming nothing of it
        self.assertNotIn(self._paths("mesh/other.ini")[0], self._linked("mesh/mod.ini"))
        self.assertEqual(self._linked("mesh/other.ini"), [])

    def test_file_isTheTextAsAdded(self):
        path = self._path("mesh/mod.ini")
        with open(path, "a") as f:
            f.write("[Later]\nx = 1\n")
        self.assertNotIn("Later", self._state.file(path).getIfTemplates())

    def test_iniFile_linkedInis_throughItsSharedState(self):
        ini = FRB.IniFile(self._path("elsewhere/far/textures.ini"))
        self.assertEqual(ini.linkedInis(), [])
        ini.sharedState = self._state
        self.assertEqual([os.path.basename(other.file) for other in ini.linkedInis()], ["mod.ini"])
