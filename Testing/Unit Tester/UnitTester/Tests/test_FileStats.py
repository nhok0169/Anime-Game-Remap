import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class FileStatsTest(BaseUnitTest):
    """
    Tests for :class:`FileStats` -- the C++-backed (pybind11) replacement for the pure-Python
    original -- the pure-Python original (renamed to ``FileStatsOld`` mid-migration) has since
    been deleted outright
    """

    def setUp(self):
        super().setUp()
        self.stats = FRB.FileStats()

    # ================================================
    # ==================== fixed =====================

    def test_addFixed_pathAdded(self):
        self.stats.addFixed("a.ini")
        self.compareSet(self.stats.fixed, {"a.ini"})

    def test_updateFixed_pathsAdded(self):
        self.stats.updateFixed({"a.ini", "b.ini"})
        self.compareSet(self.stats.fixed, {"a.ini", "b.ini"})

    # ================================================
    # =================== skipped ====================

    def test_addSkipped_realPythonExceptionPreserved(self):
        try:
            raise ValueError("boom")
        except ValueError as e:
            self.stats.addSkipped("bad.ini", e)

        self.assertIn("bad.ini", self.stats.skipped)
        self.assertIsInstance(self.stats.skipped["bad.ini"], ValueError)
        self.assertEqual(str(self.stats.skipped["bad.ini"]), "boom")

    def test_addSkipped_noModFolder_readFromFilePath(self):
        self.stats.addSkipped("C:/mods/EiRemap/bad.ini", ValueError("boom"))
        # modFolder defaults to the file's own parent directory (os.path.dirname-equivalent) --
        # the *whole* parent path, not just its last path component.
        self.assertIn("C:/mods/EiRemap", self.stats.skippedByMods)
        self.assertIn("C:/mods/EiRemap/bad.ini", self.stats.skippedByMods["C:/mods/EiRemap"])

    def test_addSkipped_explicitModFolder_used(self):
        self.stats.addSkipped("bad.ini", ValueError("boom"), modFolder = "C:/mods/EiRemap")
        self.compareDict(self.stats.skippedByMods, {"C:/mods/EiRemap": {"bad.ini": self.stats.skipped["bad.ini"]}},
                          compareValues = lambda result, expected: self.assertEqual(type(result), type(expected)))

    def test_updateSkipped_multipleEntriesAdded(self):
        e1 = ValueError("one")
        e2 = ValueError("two")
        self.stats.updateSkipped({"a.ini": e1, "b.ini": e2}, modFolder = "C:/mods")
        self.compareSet(set(self.stats.skipped.keys()), {"a.ini", "b.ini"})

    # ================================================
    # =================== removed ====================

    def test_addRemoved_pathAdded(self):
        self.stats.addRemoved("a.ini")
        self.compareSet(self.stats.removed, {"a.ini"})

    # ================================================
    # =================== undoed =====================

    def test_addUndoed_pathAdded(self):
        self.stats.addUndoed("a.ini")
        self.compareSet(self.stats.undoed, {"a.ini"})

    # ================================================
    # ============= visitedAtRemoval =================

    def test_addVisitedAtRemoval_pathAdded(self):
        self.stats.addVisitedAtRemoval("a.ini")
        self.compareSet(self.stats.visitedAtRemoval, {"a.ini"})

    # ================================================
    # ===================== clear ====================

    def test_clear_allDataCleared(self):
        self.stats.addFixed("a.ini")
        self.stats.addSkipped("b.ini", ValueError("boom"))
        self.stats.addRemoved("c.ini")
        self.stats.addUndoed("d.ini")
        self.stats.addVisitedAtRemoval("e.ini")

        self.stats.clear()

        self.compareSet(self.stats.fixed, set())
        self.compareDict(self.stats.skipped, {})
        self.compareDict(self.stats.skippedByMods, {})
        self.compareSet(self.stats.removed, set())
        self.compareSet(self.stats.undoed, set())
        self.compareSet(self.stats.visitedAtRemoval, set())

    # ================================================
    # ===================== update ====================

    def test_update_allFieldsUpdatedAtOnce(self):
        self.stats.update(modFolder = "C:/mods", newFixed = {"a.ini"}, newSkipped = {"b.ini": ValueError("boom")},
                           newRemoved = {"c.ini"}, newUndoed = {"d.ini"}, newVisitedAtRemoval = {"e.ini"})

        self.compareSet(self.stats.fixed, {"a.ini"})
        self.compareSet(set(self.stats.skipped.keys()), {"b.ini"})
        self.compareSet(self.stats.removed, {"c.ini"})
        self.compareSet(self.stats.undoed, {"d.ini"})
        self.compareSet(self.stats.visitedAtRemoval, {"e.ini"})
