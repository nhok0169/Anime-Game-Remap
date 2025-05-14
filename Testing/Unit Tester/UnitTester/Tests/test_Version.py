import sys
from unittest.mock import patch
import unittest.mock as mock
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


##########
# Note:
#    This unit test is created using AI (Github Copilot).
##########
class VersionTest(BaseUnitTest):
    def setUp(self):
        # Initialize a Version instance for testing
        self.version = FRB.Version()

    # ============== __init__ ========================

    def test_initial_initialSetCorrect(self):
        # Test the initial state of the Version instance
        self.assertEqual(self.version.versions, [])
        self.assertIsNone(self.version.latestVersion)

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

        for test in tests:
            versions = test[0]
            expectedVersions = test[1]
            expectedLatest = test[2]

            self.version.versions = versions
            self.assertEqual(self.version.versions, expectedVersions)
            self.assertEqual(self.version.latestVersion, expectedLatest)

    # ================================================
    # ============== latestVersion ===================

    def test_differentVersions_latsetVersionSet(self):
        tests = [
            ([1.0, 2.0, 3.0], 3.0),
            ([3.0, 2.0, 1.0], 3.0),
            ([2.0, 1.0, 3.0], 3.0),
            ([1.5, 2.5, 3.5], 3.5),
            ([], None),
            ([1.0], 1.0),
            ([2.0, 2.0, 2.0], 2.0),
            ([1.0, 2.0, 1.0], 2.0)
        ]

        for test in tests:
            versions = test[0]
            expectedLatest = test[1]

            self.version.versions = versions
            self.assertEqual(self.version.latestVersion, expectedLatest)

    # ================================================
    # ============== add =============================

    def test_addVersions_versionsSetCorrectly(self):
        # Test adding a new version
        self.version.add(1.0)
        self.assertEqual(self.version.versions, [1.0])
        self.assertEqual(self.version.latestVersion, 1.0)

        self.version.add(2.0)
        self.assertEqual(self.version.versions, [1.0, 2.0])
        self.assertEqual(self.version.latestVersion, 2.0)

        self.version.add(1.5)
        self.assertEqual(self.version.versions, [1.0, 1.5, 2.0])
        self.assertEqual(self.version.latestVersion, 2.0)

        self.version.add(-1.0)
        self.assertEqual(self.version.versions, [-1.0, 1.0, 1.5, 2.0])
        self.assertEqual(self.version.latestVersion, 2.0)

        self.version.add(1.0)
        self.assertEqual(self.version.versions, [-1.0, 1.0, 1.5, 2.0])
        self.assertEqual(self.version.latestVersion, 2.0)

    # ================================================
    # ============== findClosest =====================

    def test_find_closest_version(self):
        # Test finding the closest version
        self.version.versions = [1.0, 2.0, 3.0]

        self.assertEqual(self.version.findClosest(2.5), 2.0)
        self.assertEqual(self.version.findClosest(3.5), 3.0)
        self.assertEqual(self.version.findClosest(0.5), 1.0)
        self.assertEqual(self.version.findClosest(None), 3.0)

    @mock.patch('src.FixRaidenBoss2.LruCache.__getitem__', return_value=9.3)
    def test_find_closest_version_with_cache(self, m_cache_get):
        # Test finding the closest version with caching
        self.version.versions = [1.0, 2.0, 3.0]

        self.version.findClosest(2.5, fromCache=False)  # Cache miss
        self.assertEqual(self.version.findClosest(2.5, fromCache=True), 9.3)
        m_cache_get.assert_called_once_with(2.5)

        with patch.object(FRB.LruCache, '__setitem__') as mock_cache_set:
            self.assertEqual(self.version.findClosest(2.5, fromCache=False), 2.0)
            mock_cache_set.assert_called_once_with(2.5, 2.0)

    def test_add_and_find_closest(self):
        # Test adding versions and finding the closest version
        self.version.add(1.0)
        self.version.add(3.0)
        self.version.add(2.0)

        self.assertEqual(self.version.versions, [1.0, 2.0, 3.0])
        self.assertEqual(self.version.findClosest(2.5), 2.0)

    # ================================================