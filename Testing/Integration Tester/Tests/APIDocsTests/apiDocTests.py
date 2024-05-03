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

    def test_iniFileFromStr_onlyIniFileFixedPart(self):
        self.runTest("iniFileFromStr_onlyIniFileFixedPart", "iniFileFromStr_onlyIniFileFixedPart.py")

    def test_iniFileFromFilePath_fixRemovedFromIni(self):
        self.runTest("iniFileFromFilePath_fixRemovedFromIni", "iniFileFromFilePath_fixRemovedFromIni.py")

    def test_iniFileFromStr_fixRemovedFromIniStr(self):
        self.runTest("iniFileFromStr_fixRemovedFromIniStr", "iniFileFromStr_fixRemovedFromIniStr.py")

    def test_iniFileFromFilePath_prevFixRemovedAndIniFixed(self):
        self.runTest("iniFileFromFilePath_prevFixRemovedAndIniFixed", "iniFileFromFilePath_prevFixRemovedAndIniFixed.py")

    def test_iniFileFromStr_prevFixRemovedAndIniFixed(self):
        self.runTest("iniFileFromStr_prevFixRemovedAndIniFixed", "iniFileFromStr_prevFixRemovedAndIniFixed.py")

    def test_blendFromPath_blendFixed(self):
        self.runTest("blendFromPath_blendFixed", r"fixBlends\RaidenShogun\Mod\blendFromPath_blendFixed.py")

    def test_blendFromPath_fixedBytes(self):
        self.runTest("blendFromPath_fixedBytes", r"fixBlends\RaidenShogun\Mod\blendFromPath_fixedBytes.py")

    def test_fullFix_modFixed(self):
        self.runTest("fullFix_modFixed", r"fullFix\RaidenShogun\Mod\pythonScript\Run\fullFix_modFixed.py")

    def test_fullFix_modFixUndoed(self):
        self.runTest("fullFix_modFixUndoed", r"fullFix\RaidenShogun\Mod\pythonScript\Run\fullFix_modFixUndoed.py")