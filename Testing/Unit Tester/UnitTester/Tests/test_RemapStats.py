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
        # Every file kind RemapIniRemover.classifyResource sorts a removed resource into, plus 'ini'.
        # 'mod' used to be here too -- it was a mod-FOLDER tally rather than a file kind, and was
        # removed along with the reporting that consumed it.
        for name in ("blend", "position", "texcoord", "buf", "other", "ini", "texEdit", "texAdd"):
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
        self.stats.texcoord.addFixed("c")
        self.stats.buf.addFixed("d")
        self.stats.other.addFixed("e")
        self.stats.ini.addFixed("f")
        self.stats.texEdit.addFixed("g")
        self.stats.texAdd.addFixed("h")
        self.stats.download.addHit("i")

        self.stats.clear()

        for name in ("blend", "position", "texcoord", "buf", "other", "ini", "texEdit", "texAdd"):
            self.compareSet(getattr(self.stats, name).fixed, set())
        self.compareSet(self.stats.download.hit, set())

    def test_subStatsAreIndependent(self):
        self.stats.blend.addFixed("blend.buf")
        self.compareSet(self.stats.position.fixed, set())
