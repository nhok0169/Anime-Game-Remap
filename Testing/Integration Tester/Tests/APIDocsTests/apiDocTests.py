from IntegrationTester import IntegrationTest
import os


class ApiDocTests(IntegrationTest):
    # getTestPath(): Retrieves the absolute path for the tests
    def getTestPath(self) -> str:
        return os.path.dirname(os.path.abspath(__file__))


    def test_iniFileFromFilePath_iniFileFixed(self):
        self.runTest("iniFileFromFilePath_iniFileFixed", "iniFileFromFilePath_iniFileFixed.py")

    def test_iniFileFromStr_iniFileFixed(self):
        self.runTest("iniFileFromStr_iniFileFixed", "iniFileFromStr_iniFileFixed.py")

    def test_iniStr_onlyIniFixedPart(self):
        self.runTest("iniStr_onlyIniFixedPart", "iniStr_onlyIniFixedPart.py")

    def test_iniPath_iniFixRemoved(self):
        self.runTest("iniPath_iniFixRemoved", "iniPath_iniFixRemoved.py")

    def test_iniStr_iniFixRemoved(self):
        self.runTest("iniStr_iniFixRemoved", "iniStr_iniFixRemoved.py")

    def test_iniPath_prevRemovedIniFixed(self):
        self.runTest("iniPath_prevRemovedIniFixed", "iniPath_prevRemovedIniFixed.py")

    def test_iniStr_prevRemovedIniFixed(self):
        self.runTest("iniStr_prevRemovedIniFixed", "iniStr_prevRemovedIniFixed.py")

    def test_blendFromPath_blendFixed(self):
        self.runTest("blendFromPath_blendFixed", r"fixBlends\RaidenShogun\Mod\blendFromPath_blendFixed.py")

    def test_blendFromPath_fixedBytes(self):
        self.runTest("blendFromPath_fixedBytes", r"fixBlends\RaidenShogun\Mod\blendFromPath_fixedBytes.py")

    def test_fullFix_modFixed(self):
        self.runTest("fullFix_modFixed", r"fullFix\RaidenShogun\Mod\pythonScript\Run\fullFix_modFixed.py")

    def test_fullFix_modFixUndoed(self):
        self.runTest("fullFix_modFixUndoed", r"fullFix\RaidenShogun\Mod\pythonScript\Run\fullFix_modFixUndoed.py")