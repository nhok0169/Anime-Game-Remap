import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class CppModTypeMethodsTest(BaseUnitTest):
    """
    Differential tests for :class:`CppModType`'s ported methods against the pure-Python
    :class:`ModType`.

    Both sides are read at runtime and diffed across all 43 shipped mod types, rather than the
    expectations being written out here -- a table restated by hand only pins the C++ against
    itself.

    :meth:`getModsToFix` is the one deliberate divergence and is tested on its own terms below;
    everything else has to agree exactly.
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._py = {mt.name: mt for mt in FRB.ModTypes.getAll()}
        cls._cpp = {mt.name: mt for mt in FRB.CppGlobalModTypes.all()}

    def _pairs(self):
        for name in sorted(self._py):
            yield name, self._py[name], self._cpp[name]

    def test_bothSidesShipTheSameModTypes(self):
        self.compareSet(set(self._cpp.keys()), set(self._py.keys()))

    # ================================================
    # ==================== isName ====================

    def test_isName_matchesPython(self):
        for name, pyModType, cppModType in self._pairs():
            candidates = [name, name.lower(), name.upper(), "definitelyNotAModType"] + list(pyModType.aliases)
            for candidate in candidates:
                with self.subTest(modType = name, candidate = candidate):
                    self.assertEqual(cppModType.isName(candidate), pyModType.isName(candidate))

    def test_isName_isCaseInsensitive(self):
        raiden = self._cpp["Raiden"]

        self.assertTrue(raiden.isName("raiden"))
        self.assertTrue(raiden.isName("RAIDEN"))
        self.assertTrue(raiden.isName("ShOgUn"))
        self.assertFalse(raiden.isName("Amber"))

    # ================================================
    # ================ getVertexCount ================

    def test_getVertexCount_matchesPython(self):
        for name, pyModType, cppModType in self._pairs():
            with self.subTest(modType = name):
                try:
                    expected = pyModType.getVertexCount()
                except Exception:
                    # The pure-Python original raises for a mod type with no row; the C++ one
                    # answers None instead, which is the documented divergence.
                    self.assertIsNone(cppModType.getVertexCount())
                    continue

                self.assertEqual(cppModType.getVertexCount(), expected)

    def test_getVertexCount_isAnIntNotAString(self):
        # Regression guard: the asset tables' VALUE type is deliberately py::object, because
        # VertexCounts holds ints and VGRemaps holds VGRemap objects. Narrowing it to std::string
        # silently turned both into their str() form.
        self.assertIsInstance(self._cpp["Raiden"].getVertexCount(), int)
        self.assertIsInstance(self._py["Raiden"].getVertexCount(), int)

    # ================================================
    # ================== getVGRemap ==================

    def test_getVGRemap_matchesPythonAcrossEveryRemapPair(self):
        for name, pyModType, cppModType in self._pairs():
            for target in sorted(pyModType.hashes.map.get(name, [])):
                with self.subTest(modType = name, target = target):
                    cppRemap = cppModType.getVGRemap(target)
                    pyRemap = pyModType.getVGRemap(target)

                    if (pyRemap is None):
                        self.assertIsNone(cppRemap)
                        continue

                    self.assertIsNotNone(cppRemap)
                    self.compareDict(dict(cppRemap.remap), dict(pyRemap.remap))
                    self.assertEqual(cppRemap.maxIndex, pyRemap.maxIndex)

    def test_getVGRemap_isAVGRemapNotAString(self):
        self.assertIsInstance(self._cpp["Raiden"].getVGRemap("RaidenBoss"), FRB.VGRemap)
        self.assertIsInstance(self._py["Raiden"].getVGRemap("RaidenBoss"), FRB.VGRemap)

    # ================================================
    # ================== getHelpStr ==================

    def test_getHelpStr_matchesPythonByteForByte(self):
        for name, pyModType, cppModType in self._pairs():
            with self.subTest(modType = name):
                self.assertEqual(cppModType.getHelpStr(), pyModType.getHelpStr())

    def test_getHelpStr_sortsAliases(self):
        helpStr = self._cpp["Raiden"].getHelpStr()
        aliasLine = [line for line in helpStr.split("\n") if line.startswith("aliases: ")][0]
        aliases = aliasLine[len("aliases: "):].split(", ")

        self.compareList(aliases, sorted(aliases))

    # ================================================
    # ================ getModsToFix ==================

    def test_getModsToFix_readsTheRemapTargets(self):
        # THE deliberate divergence. The pure-Python original unions hashes.fixTo and
        # indices.fixTo -- two sets it declares and never populates -- so it answers an empty set
        # for every mod type. The C++ one reads the targets that actually exist.
        for name, pyModType, cppModType in self._pairs():
            with self.subTest(modType = name):
                expected = set(pyModType.hashes.map.get(name, [])) | set(pyModType.indices.map.get(name, []))

                self.compareSet(cppModType.getModsToFix(), expected)
                self.compareSet(pyModType.getModsToFix(), set())

    def test_getModsToFix_fansOutForJean(self):
        self.compareSet(self._cpp["Jean"].getModsToFix(), {"JeanCN", "JeanSea"})

    def test_getModsToFix_raidenUnionsAcrossHashesAndIndices(self):
        # Raiden remaps by hash only, so the union has to come from the hashes side alone.
        self.compareSet(self._cpp["Raiden"].getModsToFix(), {"RaidenBoss"})

    # ================================================
    # ==================== fixIni ====================

    def test_fixIni_noOpWhenTheIniIsADifferentModType(self):
        ini = FRB.IniFile(txt = "[TextureOverrideAmberBody]\nhash = abc123\n")
        ini.classify()
        self.assertEqual(ini.availableType.name, "Amber")

        before = ini.fileTxt
        self._cpp["Raiden"].fixIni(ini)

        self.assertEqual(ini.fileTxt, before)

    def test_fixIni_noOpWhenUnclassified(self):
        ini = FRB.IniFile(txt = "[SomethingElse]\n")
        ini.classify()
        self.assertIsNone(ini.availableType)

        before = ini.fileTxt
        self._cpp["Raiden"].fixIni(ini)

        self.assertEqual(ini.fileTxt, before)

    # ================================================
    # ================ getHashRanges =================

    def test_getHashRanges_returnsRanges(self):
        colouring = FRB.IfContentPartColouring()
        result = self._cpp["Raiden"].getHashRanges(colouring)

        self.assertIsInstance(result, FRB.Ranges)

    def test_getHashRanges_emptyColouringHasNoHashKey(self):
        # Nothing carries a 'hash' key, so no index range can satisfy the filter.
        colouring = FRB.IfContentPartColouring()

        self.assertEqual(len(self._cpp["Raiden"].getHashRanges(colouring).ranges), 0)
