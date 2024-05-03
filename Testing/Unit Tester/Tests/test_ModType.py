import sys
import re
from .baseUnitTest import BaseUnitTest

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


class ModTypeTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._name = "Gregor Samsa"
        cls._bossHash = "vermin1915"
        cls._check = re.compile(".*\[.*Gregor.*Samsa.*Blend\].*")
        cls._aliases = ["Vermin", "Monster", "Merchant"]
        cls._vgRemap = {0: 7, 1: 6, 2: 5, 3: 4, 4: 3, 5: 2, 6: 1, 7: 0}
        cls._modType = None

    def setupMod(self):
        self._modType = FRB.ModType(self._name, self._check, self._bossHash, aliases = self._aliases, vgRemap = self._vgRemap)


    # ====================== vgRemap.setter ==============================

    def test_newVGRemap_vgRemapSetWithMaxVgIndex(self):
        self.setupMod()

        tests = [[{}, None],
                 [{1: 2, 8: 45, -100: 45, 0: 23, 999: 888 }, 999]]

        for test in tests:
            newVGRemap = test[0]
            self._modType.vgRemap = newVGRemap 
            self.compareDict(self._modType.vgRemap, newVGRemap)
            self.assertIs(self._modType.maxVgIndex, test[1])

    # ====================================================================
    # ====================== isName ======================================

    def test_differentSearch_searchInNameOrAliases(self):
        self.setupMod()

        tests = {"Gregor Samsa": True,
                 "gReGor SamSa": True,
                 "gregor samsa ": False,
                 "Vermin": True,
                 "mOnStEr": True,
                 "  merChant ": False}
        
        for name in tests:
            result = self._modType.isName(name)
            self.assertEqual(result, tests[name])

    # ====================================================================
    # ====================== isType ======================================

    def test_regexCheck_nameMatchesRegex(self):        
        self.setupMod()

        tests = {"[Poor Gregor Samsa Coackroach Blend]": True,
                 "[gregor can't Samsa get up Blend]": False}
        
        for search in tests:
            result = self._modType.isType(search)
            self.assertEqual(result, tests[search])

    def test_strCheck_nameIsString(self):
        self._check = "Gynopedie"
        self.setupMod() 

        tests = {"Gynopedie": True,
                 "le gynopedia": False}
        
        for search in tests:
            result = self._modType.isType(search)
            self.assertEqual(result, tests[search])

    def test_funcCheck_basedOffPredicateFunction(self):
        self._check = lambda line: len(line) % 2 == 0
        self.setupMod()

        tests = {"even": True,
                 "odd": False}

        for search in tests:
            result = self._modType.isType(search)
            self.assertEqual(result, tests[search])

    # ====================================================================