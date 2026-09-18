import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CachedFileStatsTest(BaseUnitTest):
    """
    Tests for :class:`CachedFileStats` -- inherits :class:`FileStats`'s fields/methods plus its
    own :attr:`hit`
    """

    def setUp(self):
        super().setUp()
        self.stats = FRB.CachedFileStats()

    def test_isInstanceOfFileStats(self):
        self.assertIsInstance(self.stats, FRB.FileStats)

    def test_addHit_pathAdded(self):
        self.stats.addHit("a.dds")
        self.compareSet(self.stats.hit, {"a.dds"})

    def test_updateHit_pathsAdded(self):
        self.stats.updateHit({"a.dds", "b.dds"})
        self.compareSet(self.stats.hit, {"a.dds", "b.dds"})

    def test_inheritedAddFixed_stillWorks(self):
        self.stats.addFixed("a.dds")
        self.compareSet(self.stats.fixed, {"a.dds"})

    def test_clear_clearsHitAndInheritedFields(self):
        self.stats.addHit("a.dds")
        self.stats.addFixed("b.dds")

        self.stats.clear()

        self.compareSet(self.stats.hit, set())
        self.compareSet(self.stats.fixed, set())

    def test_update_hitFieldUpdated(self):
        self.stats.update(newHit = {"a.dds"}, newFixed = {"b.dds"})
        self.compareSet(self.stats.hit, {"a.dds"})
        self.compareSet(self.stats.fixed, {"b.dds"})
