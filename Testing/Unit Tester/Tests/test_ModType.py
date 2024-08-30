import sys
import re
from .baseUnitTest import BaseUnitTest

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )/api')
import src.FixRaidenBoss2.FixRaidenBoss2 as FRB


class ModTypeTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._name = "Gregor Samsa"
        cls._hashes = FRB.Hashes()
        cls._indices = FRB.Indices()
        cls._check = re.compile(".*\[.*Gregor.*Samsa.*Blend\].*")
        cls._aliases = ["Vermin", "Monster", "Merchant"]
        cls._vgRemaps = FRB.VGRemaps()
        cls._modType = None

    def setupMod(self):
        self._hashes.addMap({"gregor samsa": {"gregor samsa"}}, {0.0: {"gregor samsa": {"blend_vb": "businessman1910", "draw_vb": "cloth", "texcoord_vb": "man"}},
                                                                 1.0: {"gregor samsa": {"blend_vb": "vermin1915", "draw_vb": "nocloth"}},
                                                                 2.3: {"gregor samsa": {"blend_vb": "cockroach1915", "texcoord_vb": "horriblevermin"}}})
        
        self._vgRemaps.addMap({"gregor samsa": {"gregor samsa"}}, {0.0: {"gregor samsa": {"gregor samsa": {0: 7, 1: 6, 2: 5, 3: 4, 4: 3, 5: 2, 6: 1, 7: 0}}}})

        self._modType = FRB.ModType(self._name, self._check, self._hashes, self._indices, aliases = self._aliases, vgRemaps = self._vgRemaps)


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
    # ====================== getModsToFix ================================

    # TODO: Makes tests for retrieving  the types of mods to fix

    # ====================================================================
    # ====================== getVGRemap ==================================

    # TODO: Make tests for getting the corresponding vertex group remap

    # ====================================================================