import sys
from typing import List, Optional

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


def v(raw) -> Optional[FRB.Version]:
    return None if raw is None else FRB.Version.parse(f"{raw}")


def vs(raws: List[float]) -> List[FRB.Version]:
    return [v(raw) for raw in raws]


class VersionSetTest(BaseUnitTest):
    def setUp(self):
        super().setUp()
        self.versions = FRB.VersionSet()

    # ============== __init__ ========================

    def test_initial_isEmpty(self):
        self.assertEqual(self.versions.versions, [])
        self.assertIsNone(self.versions.latestVersion)

    def test_init_withVersions_sortedAndDeduplicated(self):
        versions = FRB.VersionSet([3.0, "1.0", FRB.Version.parse("2.0"), 3])
        self.assertEqual(versions.versions, vs([1.0, 2.0, 3.0]))
        self.assertEqual(versions.latestVersion, v(3.0))

    # ================================================
    # ============== versions.setter =================

    def test_differentVersions_versionsSetCorrectly(self):
        tests = [
            ([1.0, 2.0, 3.0], [1.0, 2.0, 3.0], 3.0),
            ([3.0, 2.0, 1.0], [1.0, 2.0, 3.0], 3.0),
            ([2.0, 1.0, 3.0], [1.0, 2.0, 3.0], 3.0),
            ([1.5, 2.5, 3.5], [1.5, 2.5, 3.5], 3.5),
            ([], [], None),
            ([1.0], [1.0], 1.0),
            ([2.0, 2.0, 2.0], [2.0], 2.0),
            ([1.0, 2.0, 1.0], [1.0, 2.0], 2.0)
        ]

        for versions, expectedVersions, expectedLatest in tests:
            self.versions.versions = versions
            self.assertEqual(self.versions.versions, vs(expectedVersions))
            self.assertEqual(self.versions.latestVersion, v(expectedLatest))

    def test_versionsSetter_invalidVersion_raisesAndKeepsTheOldVersions(self):
        self.versions.versions = [1.0, 2.0]
        with self.assertRaises(ValueError):
            self.versions.versions = [3.0, "not a version"]
        self.assertEqual(self.versions.versions, vs([1.0, 2.0]))

    # ================================================
    # ============== add =============================

    def test_addVersions_versionsSetCorrectly(self):
        self.versions.add(1.0)
        self.assertEqual(self.versions.versions, vs([1.0]))
        self.assertEqual(self.versions.latestVersion, v(1.0))

        self.versions.add(2.0)
        self.assertEqual(self.versions.versions, vs([1.0, 2.0]))
        self.assertEqual(self.versions.latestVersion, v(2.0))

        self.versions.add(1.5)
        self.assertEqual(self.versions.versions, vs([1.0, 1.5, 2.0]))
        self.assertEqual(self.versions.latestVersion, v(2.0))

        # not a PEP 440 version: ignored rather than raised, as the pure-Python class did
        self.versions.add(-1.0)
        self.assertEqual(self.versions.versions, vs([1.0, 1.5, 2.0]))
        self.assertEqual(self.versions.latestVersion, v(2.0))

        self.versions.add(1.0)
        self.assertEqual(self.versions.versions, vs([1.0, 1.5, 2.0]))
        self.assertEqual(self.versions.latestVersion, v(2.0))

    # ================================================
    # ============== clear ===========================

    def test_clear_emptiesEverything(self):
        self.versions.versions = [1.0, 2.0]
        self.versions.clear()
        self.assertEqual(self.versions.versions, [])
        self.assertIsNone(self.versions.latestVersion)
        self.assertIsNone(self.versions.findClosest(1.0))

    # ================================================
    # ============== findClosest =====================

    def test_findClosest_largestNotGreaterThanTheQuery(self):
        self.versions.versions = [1.0, 2.0, 3.0]
        self.assertEqual(self.versions.findClosest(2.5), v(2.0))
        self.assertEqual(self.versions.findClosest(3.5), v(3.0))
        self.assertEqual(self.versions.findClosest(2.0), v(2.0))
        self.assertEqual(self.versions.findClosest(0.5), v(1.0))
        self.assertEqual(self.versions.findClosest(None), v(3.0))
        self.assertEqual(self.versions.findClosest(), v(3.0))

    def test_findClosest_noVersions_isNone(self):
        self.assertIsNone(self.versions.findClosest(2.0))

    def test_findClosest_invalidVersion_raises(self):
        self.versions.versions = [1.0]
        with self.assertRaises(ValueError):
            self.versions.findClosest("not a version")

    def test_findClosest_cacheIsNotInvalidatedByAdd(self):
        # documented: add() does not invalidate the closest-version cache
        self.versions.versions = [1.0, 3.0]
        self.assertEqual(self.versions.findClosest(2.5), v(1.0))

        self.versions.add(2.0)
        self.assertEqual(self.versions.findClosest(2.5, fromCache = True), v(1.0))
        self.assertEqual(self.versions.findClosest(2.5, fromCache = False), v(2.0))
        self.assertEqual(self.versions.findClosest(2.5, fromCache = True), v(2.0))

    def test_add_and_find_closest(self):
        self.versions.add(1.0)
        self.versions.add(3.0)
        self.versions.add(2.0)

        self.assertEqual(self.versions.versions, vs([1.0, 2.0, 3.0]))
        self.assertEqual(self.versions.findClosest(2.5), v(2.0))

    # ================================================
    # ============== static helpers ==================

    def test_getVersion(self):
        self.assertEqual(FRB.VersionSet.getVersion(4.0), v(4.0))
        self.assertEqual(FRB.VersionSet.getVersion("6.1"), v(6.1))
        self.assertEqual(FRB.VersionSet.getVersion(v(5.7)), v(5.7))
        self.assertIsNone(FRB.VersionSet.getVersion("not a version"))
        self.assertIsNone(FRB.VersionSet.getVersion(None))

    def test_compareVersions(self):
        self.assertEqual(FRB.VersionSet.compareVersions(v(1.0), v(1.0)), 0)
        self.assertLess(FRB.VersionSet.compareVersions(v(1.0), v(2.0)), 0)
        self.assertGreater(FRB.VersionSet.compareVersions(v(2.0), v(1.0)), 0)

    def test_findClosestFromSortedList(self):
        versions = [1.0, 2.0, 3.0]
        self.assertEqual(FRB.VersionSet.findClosestFromSortedList(versions, 2.5), v(2.0))
        self.assertEqual(FRB.VersionSet.findClosestFromSortedList(versions, 0.5), v(1.0))
        self.assertEqual(FRB.VersionSet.findClosestFromSortedList(versions, None), v(3.0))
        self.assertIsNone(FRB.VersionSet.findClosestFromSortedList([], 1.0))

    def test_findClosestFromList(self):
        versions = [3.0, 1.0, 2.0]
        self.assertEqual(FRB.VersionSet.findClosestFromList(versions, 2.5), v(2.0))
        self.assertEqual(FRB.VersionSet.findClosestFromList(versions, None), v(3.0))
        # no fallback to the smallest version, unlike the sorted variant
        self.assertIsNone(FRB.VersionSet.findClosestFromList(versions, 0.5))
        self.assertIsNone(FRB.VersionSet.findClosestFromList([], 1.0))

    # ================================================
