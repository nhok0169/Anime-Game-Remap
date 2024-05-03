from IntegrationTester import IntegrationTest
import os


class MixedModsTest(IntegrationTest):
    # getTestPath(): Retrieves the absolute path for the tests
    def getTestPath(self) -> str:
        return os.path.dirname(os.path.abspath(__file__))
    

    def test_fullFixNotBackups_fixModsWithoutBackups(self):
        self.runTest("fullFixNoBackups_fixModsWithoutBackups", r"Mods\fullFixNoBackups_fixModsWithoutBackups.py")

    def test_fullFixNoBackupsallMods_fixAllModsWithoutBackups(self):
        self.runTest("fullFixNoBackupsallMods_fixAllModsWithoutBackups", r"Mods\fullFixNoBackupsallMods_fixAllModsWithoutBackups.py")
    

