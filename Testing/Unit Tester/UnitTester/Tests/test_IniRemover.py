import sys

from .baseIniFileTest import BaseIniFileTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.FixRaidenBoss2 as FRB


class IniRemoverTest(BaseIniFileTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._remover = None

    def createRemover(self):
        self._remover = FRB.IniRemover(self._iniFile)

    def create(self):
        self.createIniFile()
        self.createRemover()
        self._iniFile._iniRemover = self._remover

    # ====================== _removeScriptFix ============================

    def test_scriptSections_sectionsRemoved(self):
        self.create()

        tests = {"Hello": "Hello",
                 "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -----------------------------------------------": "",
                 "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -------------------------------------": "; --------------- Raiden Boss Fix ---------------\n\nFDFDFDFDF\n\n; -------------------------------------"}

        for testInput in tests:
            expected = tests[testInput]

            self._iniFile.fileTxt = testInput
            result = self._remover._removeScriptFix()
            self.assertEqual(result, expected)

        # TODO: Add case of needing to parse blend.buf files

    # ====================================================================
    # ====================== _removeFixSections ==========================

    # TODO: Add tests for removing the sections with the RemapBlend/RemapFix keywords


    # ====================================================================