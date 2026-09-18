import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BaseIniFixerTest(BaseUnitTest):
    """
    Tests for :class:`FRB.BaseIniFixer`, the C++-backed base every .ini fixer inherits from.

    .. note::
        Like :class:`FRB.BaseIniParser`, this class carries two pieces of pure-Python state the
        C++ core has no counterpart for -- ``_parser`` and ``_iniFile`` -- because real callers
        reach straight into them. Those get the bulk of the coverage here; the fix surface itself
        is a documented no-op on the base.
    """

    class _FakeIni():
        """A stand-in for an ``IniFile`` -- ``BaseIniFixer`` only ever stores it"""

        def __init__(self):
            self.sectionIfTemplates = {}
            self._isFixed = False

    class _FakeParser():
        """A stand-in for a parser -- ``BaseIniFixer`` only reads ``_iniFile`` off it"""

        def __init__(self, iniFile):
            self._iniFile = iniFile

    # =========================== construction =====================================

    def test_construct_withParser_parserAndIniFileAreTheSameObjects(self):
        ini = self._FakeIni()
        parser = self._FakeParser(ini)
        fixer = FRB.BaseIniFixer(parser)

        # Identity, not equality -- GIMIFixer.getFix and IniFixBuilder.build both read these back
        # off the fixer and expect the caller's own objects.
        self.assertIs(fixer._parser, parser)
        self.assertIs(fixer._iniFile, ini)

    def test_construct_noArgs_parserAndIniFileAreNone(self):
        fixer = FRB.BaseIniFixer()
        self.assertIsNone(fixer._parser)
        self.assertIsNone(fixer._iniFile)

    def test_construct_parserWithoutIniFile_iniFileIsNone(self):
        # A parser that isn't one of this project's own still constructs a usable fixer.
        fixer = FRB.BaseIniFixer(object())
        self.assertIsNone(fixer._iniFile)

    def test_construct_realParser_iniFileTakenFromIt(self):
        ini = self._FakeIni()
        parser = FRB.BaseIniParser(ini)
        fixer = FRB.BaseIniFixer(parser)

        self.assertIs(fixer._parser, parser)
        self.assertIs(fixer._iniFile, ini)

    # =========================== _parser / _iniFile =====================================

    def test_setParser_reassignable(self):
        fixer = FRB.BaseIniFixer()
        parser = self._FakeParser(self._FakeIni())

        fixer._parser = parser
        self.assertIs(fixer._parser, parser)

    def test_setIniFile_reassignable(self):
        fixer = FRB.BaseIniFixer()
        ini = self._FakeIni()

        fixer._iniFile = ini
        self.assertIs(fixer._iniFile, ini)

    # =========================== clear / fix =====================================

    def test_clear_isANoOpThatKeepsTheParserAndIniFile(self):
        ini = self._FakeIni()
        parser = self._FakeParser(ini)
        fixer = FRB.BaseIniFixer(parser)

        fixer.clear()

        self.assertIs(fixer._parser, parser)
        self.assertIs(fixer._iniFile, ini)

    def test_fix_isANoOpAndReturnsNone(self):
        fixer = FRB.BaseIniFixer(self._FakeParser(self._FakeIni()))
        self.assertIsNone(fixer.fix())

    def test_fix_acceptsTheKeywordArgumentsIniFileCallsItWith(self):
        # IniFile._fix calls fixer.fix(keepBackup = ..., fixOnly = ..., hideOrig = ...) by keyword.
        fixer = FRB.BaseIniFixer(self._FakeParser(self._FakeIni()))
        self.assertIsNone(fixer.fix(keepBackup = False, fixOnly = True, hideOrig = True))
