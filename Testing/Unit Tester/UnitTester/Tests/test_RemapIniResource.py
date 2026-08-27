import os
import sys
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RemapIniResourceTest(BaseUnitTest):
    """
    Tests for :class:`RemapIniResource`/:class:`RemapIniFixResource` -- inherit
    :class:`IniResource`/:class:`IniFixResource` plus :class:`RemapIniResourceMixin`
    """

    def test_remapIniResource_isInstanceOfBothBases(self):
        r = FRB.RemapIniResource("blend", "C:/mods/EiRemap", "EiBlend.buf")
        self.assertIsInstance(r, FRB.IniResource)
        self.assertIsInstance(r, FRB.RemapIniResourceMixin)

    def test_remapIniResource_hasRequired_true(self):
        r = FRB.RemapIniResource("blend", "C:/mods/EiRemap", "EiBlend.buf")
        self.assertTrue(r.hasRequired())

    def test_remapIniFixResource_isInstanceOfBothBases(self):
        r = FRB.RemapIniFixResource("blend", "C:/mods/EiRemap", "EiBlend.buf", "RaidenBlend.buf")
        self.assertIsInstance(r, FRB.IniFixResource)
        self.assertIsInstance(r, FRB.RemapIniResourceMixin)

    def test_remapIniFixResource_hasRequired_true(self):
        r = FRB.RemapIniFixResource("blend", "C:/mods/EiRemap", "EiBlend.buf", "RaidenBlend.buf")
        self.assertTrue(r.hasRequired())


class RemapIniDownloadTest(BaseUnitTest):
    """
    Tests for :class:`RemapIniDownload` -- unlike the deprecated pure-Python original, this class
    does not accept a ``Mod`` object anywhere; :meth:`fix`'s progress-reporting callbacks
    (``downloadHandler``/``cacheHitHandler``) are supplied by the caller directly instead
    """

    def setUp(self):
        super().setUp()
        self._tmpDir = tempfile.TemporaryDirectory()
        self.tmpPath = self._tmpDir.name
        self.addCleanup(self._tmpDir.cleanup)

    def _makeSrcFile(self, name: str, content: str) -> str:
        path = os.path.join(self.tmpPath, name)
        with open(path, "w", encoding = "utf-8") as f:
            f.write(content)
        return path

    def _fileUrl(self, path: str) -> str:
        return "file:///" + path.replace("\\", "/")

    def test_isInstanceOfRemapIniResource(self):
        download = FRB.FileDownload("http://example.com/a.dds", "a.dds")
        rid = FRB.RemapIniDownload("C:/mods/EiRemap", "a.dds", download)
        self.assertIsInstance(rid, FRB.RemapIniResource)

    def test_fix_realEndToEndDownload(self):
        src = self._makeSrcFile("src.txt", "some content")
        dstFolder = os.path.join(self.tmpPath, "dst")
        os.makedirs(dstFolder, exist_ok = True)

        download = FRB.FileDownload(self._fileUrl(src), "d.txt")
        rid = FRB.RemapIniDownload(dstFolder, "d.txt", download)
        downloadStats = FRB.CachedFileStats()

        result = rid.fix(downloadStats)

        self.assertTrue(result)
        self.assertTrue(os.path.isfile(os.path.join(dstFolder, "d.txt")))
        self.compareSet(downloadStats.fixed, {rid.srcPath})

    def test_remapFix_downloadHandlerInvoked(self):
        # downloadHandler/cacheHitHandler are only on remapFix() (which takes the full
        # RemapStats), not fix() (which takes just CachedFileStats) -- see this class's own
        # pybind doc comment.
        src = self._makeSrcFile("src.txt", "some content")
        dstFolder = os.path.join(self.tmpPath, "dst")
        os.makedirs(dstFolder, exist_ok = True)

        download = FRB.FileDownload(self._fileUrl(src), "d.txt")
        rid = FRB.RemapIniDownload(dstFolder, "d.txt", download)
        stats = FRB.RemapStats()

        calls = []
        rid.remapFix(stats, downloadHandler = lambda path: calls.append(path))

        self.assertEqual(len(calls), 1)
        self.compareSet(stats.download.fixed, {rid.srcPath})

    def test_remapFix_cacheHitHandlerInvokedOnSecondCall(self):
        # Reuses the *same* RemapIniDownload instance across two remapFix() calls -- a
        # RemapIniDownload takes ownership of its FileDownload on construction (disown-on-
        # construction, matching every other pybind-bound owning-resource class in this port), so
        # a fresh FileDownload/RemapIniDownload can't be constructed a second time from the same
        # Python download object. Calling remapFix() twice on the same instance is how its
        # internal FileDownload's own cache (_prevPath) actually gets exercised.
        src = self._makeSrcFile("src.txt", "some content")
        dstFolder = os.path.join(self.tmpPath, "dst")
        os.makedirs(dstFolder, exist_ok = True)

        download = FRB.FileDownload(self._fileUrl(src), "d.txt")
        rid = FRB.RemapIniDownload(dstFolder, "d.txt", download)

        rid.remapFix(FRB.RemapStats())

        cacheHits = []
        rid.remapFix(FRB.RemapStats(), cacheHitHandler = lambda path: cacheHits.append(path))

        self.assertEqual(len(cacheHits), 1)
