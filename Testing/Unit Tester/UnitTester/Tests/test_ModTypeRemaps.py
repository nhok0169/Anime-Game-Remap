import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class ModTypeRemapsTest(BaseUnitTest):
    """
    Differential tests for the remap graph -- which mod types each mod type can be fixed onto.

    The C++ table (``ModTypeIdTools.getHashRemapTargets`` / ``getIndexRemapTargets``, feeding
    ``CppGIBuilder``'s ``Hashes``/``Indices`` maps) has to agree with the pure-Python
    ``ModTypes``, which is still the live source of truth. Rather than restate the table here --
    which would only pin the C++ against itself -- these read BOTH sides at runtime and diff them,
    so the day the Python data changes the mismatch shows up as a failure rather than as a silently
    stale copy.
    """

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._pyModTypes = {mt.name: mt for mt in FRB.ModTypes.getAll()}

    def _idOf(self, name):
        found = None
        for raw in range(200):
            modTypeId = FRB.ModTypeIdTools.getEnum(raw)
            if (modTypeId is not None and FRB.ModTypeIdTools.getName(modTypeId) == name):
                found = modTypeId
                break

        self.assertIsNotNone(found, f"no ModTypeId goes by the name '{name}'")
        return found

    def _pyTargets(self, modType, assetName):
        # The map is keyed by the mod type's own name; an absent key means no targets at all.
        assetMap = dict(getattr(modType, assetName).map)
        return sorted(assetMap.get(modType.name, []))

    def _cppTargets(self, modTypeId, getter):
        return sorted(FRB.ModTypeIdTools.getName(t) for t in getter(modTypeId))

    # ================================================
    # ============== the table itself ================

    def test_hashRemapTargets_matchPython(self):
        for name, pyModType in self._pyModTypes.items():
            with self.subTest(modType = name):
                self.assertEqual(self._cppTargets(self._idOf(name), FRB.ModTypeIdTools.getHashRemapTargets),
                                 self._pyTargets(pyModType, "hashes"))

    def test_indexRemapTargets_matchPython(self):
        for name, pyModType in self._pyModTypes.items():
            with self.subTest(modType = name):
                self.assertEqual(self._cppTargets(self._idOf(name), FRB.ModTypeIdTools.getIndexRemapTargets),
                                 self._pyTargets(pyModType, "indices"))

    def test_raiden_remapsByHashOnly(self):
        # The one asymmetry in the whole table, pinned on its own so a regression names itself
        # rather than showing up as one row of a 43-row subTest sweep.
        raiden = self._idOf("Raiden")

        self.compareList(self._cppTargets(raiden, FRB.ModTypeIdTools.getHashRemapTargets), ["RaidenBoss"])
        self.compareList(FRB.ModTypeIdTools.getIndexRemapTargets(raiden), [])

    def test_bossIds_areTargetsOnlyNeverSources(self):
        for name in ("RaidenBoss", "ArlecchinoBoss"):
            with self.subTest(modType = name):
                modTypeId = self._idOf(name)
                self.compareList(FRB.ModTypeIdTools.getHashRemapTargets(modTypeId), [])
                self.compareList(FRB.ModTypeIdTools.getIndexRemapTargets(modTypeId), [])

    # ================================================
    # ============ what GIBuilder builds =============

    def test_cppGIBuilder_buildsTheSameModTypesAsPython(self):
        cppNames = {mt.name for mt in FRB.CppGlobalModTypes.all()}
        self.compareSet(cppNames, set(self._pyModTypes.keys()))

    def test_bossIds_haveNoFactory(self):
        cppNames = {mt.name for mt in FRB.CppGlobalModTypes.all()}

        self.assertNotIn("RaidenBoss", cppNames)
        self.assertNotIn("ArlecchinoBoss", cppNames)

    # ================================================
    # ================ the registry ==================

    def test_registerAll_populatesTheRegistry(self):
        FRB.ModTypeIdTools.clear()
        self.addCleanup(FRB.ModTypeIdTools.clear)

        raidenId = self._idOf("Raiden")

        # Registration is deliberately explicit -- nothing in the core does it for you.
        self.assertIsNone(FRB.ModTypeIdTools.getModType(int(raidenId)))

        FRB.CppGlobalModTypes.registerAll()

        registered = FRB.ModTypeIdTools.getModType(int(raidenId))
        self.assertIsNotNone(registered)
        self.assertEqual(registered.name, "Raiden")

    def test_registerAll_makesNamesAndAliasesFindable(self):
        FRB.ModTypeIdTools.clear()
        self.addCleanup(FRB.ModTypeIdTools.clear)
        FRB.CppGlobalModTypes.registerAll()

        self.assertEqual(FRB.ModTypeIdTools.findByName("Raiden"), self._idOf("Raiden"))
        self.assertEqual(FRB.ModTypeIdTools.findByName("Shogun"), self._idOf("Raiden"))

    def test_registerAll_skipsTheTargetOnlyIds(self):
        FRB.ModTypeIdTools.clear()
        self.addCleanup(FRB.ModTypeIdTools.clear)
        FRB.CppGlobalModTypes.registerAll()

        # Nothing builds them, so nothing registers them.
        self.assertIsNone(FRB.ModTypeIdTools.getModType(int(self._idOf("RaidenBoss"))))

    def test_registerAll_isIdempotent(self):
        FRB.ModTypeIdTools.clear()
        self.addCleanup(FRB.ModTypeIdTools.clear)

        FRB.CppGlobalModTypes.registerAll()
        FRB.CppGlobalModTypes.registerAll()

        self.assertIsNotNone(FRB.ModTypeIdTools.getModType(int(self._idOf("Raiden"))))
