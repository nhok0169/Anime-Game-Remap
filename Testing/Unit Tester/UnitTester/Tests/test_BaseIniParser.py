import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BaseIniParserTest(BaseUnitTest):
    """
    Tests for :class:`FRB.BaseIniParser`, the C++-backed base every .ini parser inherits from.

    .. note::
        This class carries two pieces of pure-Python state the C++ core deliberately does not have
        (``_iniFile`` and ``_modsToFix``) -- both exist because real callers reach straight into
        them (``BaseIniFixer.__init__``, ``MultiModFixer.fix``), so they get the bulk of the
        coverage here. The parse/clear surface is a documented no-op on the base itself.
    """

    class _FakeIni():
        """A stand-in for an ``IniFile`` -- ``BaseIniParser`` only ever stores it"""

        def __init__(self):
            self.sectionIfTemplates = {}

    # =========================== construction =====================================

    def test_construct_withIni_iniFileIsTheSameObject(self):
        ini = self._FakeIni()
        parser = FRB.BaseIniParser(ini)

        # Identity, not equality -- BaseIniFixer.__init__ does `self._iniFile = parser._iniFile`
        # and every later read has to see the caller's own object.
        self.assertIs(parser._iniFile, ini)

    def test_construct_noArgs_iniFileIsNone(self):
        parser = FRB.BaseIniParser()
        self.assertIsNone(parser._iniFile)

    def test_construct_modsToFixStartsEmptySet(self):
        parser = FRB.BaseIniParser(self._FakeIni())
        self.assertIsInstance(parser._modsToFix, set)
        self.compareSet(parser._modsToFix, set())

    def test_construct_twoParsers_doNotShareModsToFix(self):
        # A mutable-default-argument style bug would show up exactly here.
        first = FRB.BaseIniParser()
        second = FRB.BaseIniParser()

        first._modsToFix.add("Raiden")
        self.compareSet(second._modsToFix, set())

    # =========================== _iniFile / _modsToFix =====================================

    def test_setIniFile_reassignable(self):
        parser = FRB.BaseIniParser()
        ini = self._FakeIni()

        parser._iniFile = ini
        self.assertIs(parser._iniFile, ini)

    def test_setModsToFix_keepsTheAssignedObject(self):
        parser = FRB.BaseIniParser()
        mods = {"Raiden", "Jean"}

        parser._modsToFix = mods
        self.assertIs(parser._modsToFix, mods)

    def test_modsToFix_inPlaceMutationSticks(self):
        parser = FRB.BaseIniParser()
        parser._modsToFix.add("Raiden")
        self.compareSet(parser._modsToFix, {"Raiden"})

    # =========================== clear =====================================

    def test_clear_emptiesModsToFix_keepsIniFile(self):
        ini = self._FakeIni()
        parser = FRB.BaseIniParser(ini)
        parser._modsToFix = {"Raiden", "Jean"}

        parser.clear()

        self.compareSet(parser._modsToFix, set())
        self.assertIs(parser._iniFile, ini)

    def test_clear_emptiesInPlace_notByRebinding(self):
        parser = FRB.BaseIniParser()
        mods = parser._modsToFix
        mods.add("Raiden")

        parser.clear()

        # MultiModFixer hands its own set in and reads it back out afterwards, so clear() must not
        # silently swap in a different object.
        self.assertIs(parser._modsToFix, mods)
        self.compareSet(mods, set())

    # =========================== parse =====================================

    def test_parse_isANoOpAndReturnsNoGraphGroups(self):
        parser = FRB.BaseIniParser(self._FakeIni())

        # The base parses nothing, so there is nothing to hand back -- but the *shape* is still the
        # List[IniGraphGroup] every parser returns, not None.
        result = parser.parse()
        self.assertIsInstance(result, list)
        self.compareList(result, [])
