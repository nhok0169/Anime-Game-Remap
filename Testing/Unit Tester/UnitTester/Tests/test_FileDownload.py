import os
import sys
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class FileDownloadTest(BaseUnitTest):
    """
    Tests for :class:`FileDownload` -- the C++-backed (real libcurl) replacement for the
    pure-Python original -- the original (renamed to ``FileDownloadOld`` mid-migration) has since
    been deleted outright :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        Deliberately does **not** inherit :class:`BaseFileUnitTest` -- that class mocks
        :mod:`os`/:mod:`shutil` at the Python level, which the new C++-backed download path never
        goes through (real libcurl/filesystem calls from C++). Uses a real
        :class:`tempfile.TemporaryDirectory` instead, matching this port's own
        ``core/tests/FileDownload_curl_test.cpp`` precedent
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

    # ================================================
    # ===================== get ======================

    def test_get_freshDownload_fileWrittenAndTupleCorrect(self):
        src = self._makeSrcFile("src.txt", "hello world")
        dstFolder = os.path.join(self.tmpPath, "dst")
        os.makedirs(dstFolder, exist_ok = True)

        download = FRB.FileDownload(self._fileUrl(src), "downloaded.txt")
        path, downloaded, wasDownloaded = download.get(dstFolder)

        self.assertTrue(downloaded)
        with open(path, encoding = "utf-8") as f:
            self.assertEqual(f.read(), "hello world")

    def test_get_cacheHit_secondCallCopiesInsteadOfRedownloading(self):
        src = self._makeSrcFile("src.txt", "hello world")
        dst1 = os.path.join(self.tmpPath, "dst1")
        dst2 = os.path.join(self.tmpPath, "dst2")
        os.makedirs(dst1, exist_ok = True)
        os.makedirs(dst2, exist_ok = True)

        download = FRB.FileDownload(self._fileUrl(src), "downloaded.txt", cache = True)
        download.get(dst1)
        path2, downloaded2, _ = download.get(dst2)

        self.assertFalse(downloaded2)
        with open(path2, encoding = "utf-8") as f:
            self.assertEqual(f.read(), "hello world")

    def test_get_cacheDisabled_alwaysRedownloads(self):
        src = self._makeSrcFile("src.txt", "hello world")
        dst1 = os.path.join(self.tmpPath, "dst1")
        dst2 = os.path.join(self.tmpPath, "dst2")
        os.makedirs(dst1, exist_ok = True)
        os.makedirs(dst2, exist_ok = True)

        download = FRB.FileDownload(self._fileUrl(src), "downloaded.txt", cache = False)
        download.get(dst1)
        _, downloaded2, _ = download.get(dst2)

        self.assertTrue(downloaded2)

    # ================================================
    # =================== download ===================

    def test_download_returnsFullPathAndWritesFile(self):
        src = self._makeSrcFile("src.txt", "some content")
        dstFolder = os.path.join(self.tmpPath, "dst")
        os.makedirs(dstFolder, exist_ok = True)

        download = FRB.FileDownload(self._fileUrl(src), "out.txt")
        path = download.download(dstFolder)

        with open(path, encoding = "utf-8") as f:
            self.assertEqual(f.read(), "some content")

    def test_download_unreachableUrl_raisesAndLeavesNoPartialFile(self):
        dstFolder = os.path.join(self.tmpPath, "dst")
        os.makedirs(dstFolder, exist_ok = True)

        download = FRB.FileDownload(self._fileUrl(os.path.join(self.tmpPath, "does_not_exist.txt")), "out.txt")
        with self.assertRaises(Exception):
            download.download(dstFolder)

        self.assertFalse(os.path.isfile(os.path.join(dstFolder, "out.txt")))
