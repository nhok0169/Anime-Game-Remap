from ...src.IntegrationTest import IntegrationTest
from ordered_set import OrderedSet
from collections import OrderedDict
import os


class MixedModsTest(IntegrationTest):
    # getTestPath(): Retrieves the absolute path for the tests
    def getTestPath(self) -> str:
        return os.path.dirname(os.path.abspath(__file__))
    
    def setUp(self):
        self.patch("builtins.set", OrderedSet)
        self.patch("builtins.dict", OrderedDict)


    def test_fullFixNotBackups_fixModsWithoutBackups(self):
        self.runTest("fullFixNoBackups_fixModsWithoutBackups", r"Mods\fullFixNoBackups_fixModsWithoutBackups.py")

    def test_fullFixNoBackupsallMods_fixAllModsWithoutBackups(self):
        self.runTest("fullFixNoBackupsallMods_fixAllModsNoBackups", r"Mods\fullFixNoBackupsallMods_fixAllModsNoBackups.py")

    def test_fullFixAllModsFixOnly_fixAllModsPrevChangeStays(self):
        self.runTest("fullFixAllModsFixOnly_fixAllMods", r"Mods\fullFixAllModsFixOnly_fixAllMods.py")

    def test_fixAllModsFixOnlyInisUndoed_fixAllModsPrevChangeStays(self):
        self.runTest("fixAllModsFixOnlyInisUndoed_fixedAllMods", r"Mods\fixAllModsFixOnlyInisUndoed_fixedAllMods.py")

    def test_fixAllModsAndUndoFix_filesSameAsBefore(self):
        self.runTest("fixAllModsAndUndoFix_filesSameAsBefore", r"Mods\fixAllModsAndUndoFix_filesSameAsBefore.py")
    

