import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class RemapStatsTest(BaseUnitTest):
    """
    Tests for :class:`RemapStats` -- the aggregate stats object used by the overall remap process,
    whose sub-attributes are :class:`FileStats`/:class:`CachedFileStats` instances (not plain
    :class:`RemapStats` itself, matching what the actual remap process needs -- see this class's
    own pybind binding's doc comment)
    """

    def setUp(self):
        super().setUp()
        self.stats = FRB.RemapStats()

    def test_defaultConstruction_subStatsAreCorrectTypes(self):
        for name in ("blend", "position", "ini", "mod", "texEdit", "texAdd"):
            self.assertIsInstance(getattr(self.stats, name), FRB.FileStats)

        self.assertIsInstance(self.stats.download, FRB.CachedFileStats)

    def test_mutateSubStat_persistsOnReread(self):
        self.stats.blend.addFixed("blend.buf")
        self.compareSet(self.stats.blend.fixed, {"blend.buf"})

    def test_mutateDownloadSubStat_hitPersists(self):
        self.stats.download.addHit("some.dds")
        self.compareSet(self.stats.download.hit, {"some.dds"})

    def test_clear_clearsEverySubStat(self):
        self.stats.blend.addFixed("a")
        self.stats.position.addFixed("b")
        self.stats.ini.addFixed("c")
        self.stats.mod.addFixed("d")
        self.stats.texEdit.addFixed("e")
        self.stats.texAdd.addFixed("f")
        self.stats.download.addHit("g")

        self.stats.clear()

        for name in ("blend", "position", "ini", "mod", "texEdit", "texAdd"):
            self.compareSet(getattr(self.stats, name).fixed, set())
        self.compareSet(self.stats.download.hit, set())

    def test_subStatsAreIndependent(self):
        self.stats.blend.addFixed("blend.buf")
        self.compareSet(self.stats.position.fixed, set())
