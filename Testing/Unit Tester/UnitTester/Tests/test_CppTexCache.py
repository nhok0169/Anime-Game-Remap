import gc
import os
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppTexCacheTest(BaseUnitTest):
    """
    Tests for :class:`TexCache` -- the run-level texture cache :class:`RemapService` hands to every
    texture resource, and which a prototype driving :class:`TextureFile` directly can share itself

    The behaviour that matters is that it answers on CONTENT and never on identity: a filter chain
    is an arbitrary callable and cannot be inspected, so the decode half is keyed on the source
    file's bytes and the write half on the finished pixels. A cache that answered on the PATH would
    hand one texture's bytes to another object the moment two mods held the same file under
    different names
    """

    def setUp(self):
        super().setUp()
        self._tmpFiles = []

    def tearDown(self):
        for path in self._tmpFiles:
            if (os.path.isfile(path)):
                os.remove(path)
        super().tearDown()

    def _tmpPath(self, name):
        path = os.path.join(tempfile.gettempdir(), name)
        self._tmpFiles.append(path)
        if (os.path.isfile(path)):
            os.remove(path)
        return path

    def _writeTexture(self, path, colour = (10, 20, 30, 255), width = 8, height = 8):
        """A real, readable .dds on disk -- the cache is only reachable through open()/save()"""

        texFile = FRB.TextureFile(path)
        texFile.setPixels(bytes(bytearray(colour * width * height)), width, height)
        texFile.save(None, False, False)
        return path

    def test_newCacheCountsNothing(self):
        cache = FRB.TexCache()

        self.assertEqual(cache.getDecodeHits(), 0)
        self.assertEqual(cache.getWriteHits(), 0)
        self.assertEqual(cache.getBytes(), 0)

    def test_textureHasNoCacheByDefault(self):
        """The cache is opt-in -- every caller before it existed must keep the behaviour it had"""

        self.assertIsNone(FRB.TextureFile("nothing.dds").getCache())

    def test_setCache_isReadBack(self):
        cache = FRB.TexCache()
        texFile = FRB.TextureFile("nothing.dds")
        texFile.setCache(cache)

        self.assertIsNotNone(texFile.getCache())

    def test_sameSourceIsDecodedOnce(self):
        src = self._writeTexture(self._tmpPath("AGRemapTexCacheSame.dds"))
        cache = FRB.TexCache()

        for _ in range(3):
            texFile = FRB.TextureFile(src)
            texFile.setCache(cache)
            texFile.open()

        self.assertEqual(cache.getDecodeMisses(), 1, "the first open is the only real decode")
        self.assertEqual(cache.getDecodeHits(), 2, "the other two came from the cache")

    def test_differentSourcesAreBothDecoded(self):
        """The counterpart that actually protects correctness -- a hit must need the same bytes"""

        first = self._writeTexture(self._tmpPath("AGRemapTexCacheA.dds"), (10, 20, 30, 255))
        second = self._writeTexture(self._tmpPath("AGRemapTexCacheB.dds"), (200, 100, 50, 255))
        cache = FRB.TexCache()

        pixels = []
        for src in (first, second):
            texFile = FRB.TextureFile(src)
            texFile.setCache(cache)
            texFile.open()
            pixels.append(texFile.getPixels())

        self.assertEqual(cache.getDecodeHits(), 0, "different files must not hit each other")
        self.assertNotEqual(pixels[0], pixels[1], "and each must keep its own pixels")

    def test_identicalCopiesUnderDifferentNamesDoHit(self):
        """Keyed on content, not path: two mods routinely ship the same texture under two names"""

        first = self._writeTexture(self._tmpPath("AGRemapTexCacheCopy1.dds"))
        second = self._tmpPath("AGRemapTexCacheCopy2.dds")
        with open(first, "rb") as handle:
            data = handle.read()
        with open(second, "wb") as handle:
            handle.write(data)

        cache = FRB.TexCache()
        for src in (first, second):
            texFile = FRB.TextureFile(src)
            texFile.setCache(cache)
            texFile.open()

        self.assertEqual(cache.getDecodeHits(), 1, "a byte-identical copy under another name hits")

    def test_sameOutputIsWrittenOnceAndCopiedAfter(self):
        src = self._writeTexture(self._tmpPath("AGRemapTexCacheWriteSrc.dds"))
        cache = FRB.TexCache()

        written = []
        for i in range(3):
            dest = self._tmpPath("AGRemapTexCacheOut{}.dds".format(i))
            texFile = FRB.TextureFile(src)
            texFile.setCache(cache)
            texFile.open()
            texFile.saveAs(dest)
            written.append(dest)

        # saveAs on a .dds compresses, so this exercises the expensive path the cache exists for
        self.assertEqual(cache.getWriteMisses(), 1, "only the first write really encodes")
        self.assertEqual(cache.getWriteHits(), 2, "the rest are copies")

        contents = []
        for dest in written:
            with open(dest, "rb") as handle:
                contents.append(handle.read())

        self.assertEqual(contents[0], contents[1], "a copied output is identical to what it copied")
        self.assertEqual(contents[0], contents[2])

    def test_zeroBudgetDisablesTheDecodeHalf(self):
        src = self._writeTexture(self._tmpPath("AGRemapTexCacheNoBudget.dds"))
        cache = FRB.TexCache(0)

        for _ in range(2):
            texFile = FRB.TextureFile(src)
            texFile.setCache(cache)
            texFile.open()

        self.assertEqual(cache.getDecodeHits(), 0, "nothing is held, so nothing can hit")

    def test_cacheSurvivesHavingNoPythonReference(self):
        """
        keep_alive on setCache. The texture holds a BORROWED pointer, so a cache Python has dropped
        would otherwise be collected while the texture still reads through it
        """

        src = self._writeTexture(self._tmpPath("AGRemapTexCacheAlive.dds"))

        texFile = FRB.TextureFile(src)
        texFile.setCache(FRB.TexCache())
        gc.collect()

        texFile.open()

        self.assertTrue(texFile.hasImage, "the texture still opened after its cache lost its name")
        self.assertEqual(texFile.getCache().getDecodeMisses(), 1, "and the cache is still usable")
