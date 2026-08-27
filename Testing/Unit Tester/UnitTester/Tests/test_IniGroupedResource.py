import copy
import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IniGroupedResourceTest(BaseUnitTest):
    """
    Tests for :class:`IniGroupedResource` -- the C++-backed replacement for the pure-Python
    original -- the pure-Python original (renamed to ``IniGroupedResourceOld`` mid-migration) has
    since been deleted outright :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        :attr:`resources` is deliberately a general-purpose Python ``dict`` (arbitrary hashable
        keys/values), not a typed ``Dict[str, IniResource]`` -- this matches how
        :class:`ResGroupCollect` actually uses it as scratch storage during its multi-phase
        collection algorithm (mod-object tuple keys, placeholder tuple values that later get
        replaced with real resource objects, and ``copy.deepcopy()``'d mid-algorithm). See
        :class:`IniGroupedResource`'s own binding source comment for the full rationale
    """

    def test_defaultConstruction_emptyResourcesAndBuilt(self):
        g = FRB.IniGroupedResource("blendGroup")
        self.assertEqual(g.name, "blendGroup")
        self.assertTrue(g.isBuilt)
        self.compareDict(g.resources, {})

    def test_fix_noFixFunc_isNoOp(self):
        g = FRB.IniGroupedResource("blendGroup")
        self.assertFalse(bool(g.fix()))

    def test_fix_withFixFunc_invokedFromPython(self):
        calls = []

        def fixFunc(self):
            calls.append(self.name)
            return True

        r = FRB.IniResource("blend", "C:/mods/EiRemap", "EiBlend.buf")
        g = FRB.IniGroupedResource("blendGroup", {"blend": r}, fixFunc, True)
        result = g.fix()

        self.assertTrue(result)
        self.assertEqual(calls, ["blendGroup"])

    # ================================================
    # ================ resources dict ================
    # (real ResGroupCollect-style usage: tuple keys, placeholder tuple values)

    def test_resources_tupleKeyAndPlaceholderTupleValue_roundTrip(self):
        g = FRB.IniGroupedResource("group1")
        resModObj = (0, "comp", "obj")
        g.resources[resModObj] = ("fileKey1", "rootSection", None, 3)

        self.assertIn(resModObj, g.resources)
        fileKey, rootSectionName, rootLocation, partDepth = g.resources[resModObj]
        self.assertEqual(fileKey, "fileKey1")
        self.assertEqual(partDepth, 3)

    def test_resources_lateSubstitutionWithRealResource(self):
        # Mirrors ResGroupCollect._connectResGroups: a placeholder tuple gets overwritten with a
        # real IniResource instance at the same key.
        g = FRB.IniGroupedResource("group1")
        resModObj = (0, "comp", "obj")
        g.resources[resModObj] = ("fileKey1", "rootSection", None, 0)

        realResource = FRB.IniResource("blend", "C:/mods/EiRemap", "EiBlend.buf")
        g.resources[resModObj] = realResource

        self.assertIs(g.resources[resModObj], realResource)

    def test_addResource_genericKeyAndValue(self):
        g = FRB.IniGroupedResource("group1")
        resModObj = (0, "comp", "obj")
        g.addResource(resModObj, ("fileKey1", "rootSection", None, 0))
        self.assertIn(resModObj, g.resources)

    def test_isMissing_tupleKeyedCollectedSet(self):
        g = FRB.IniGroupedResource("group1")
        resModObj = (0, "comp", "obj")
        g.resources[resModObj] = ("fileKey1", "rootSection", None, 0)

        self.assertFalse(g.isMissing({resModObj}))
        self.assertTrue(g.isMissing({(1, "other", "obj")}))
        self.assertFalse(g.isMissing(set()))

    def test_isMissing_stringKeyedCollectedSet_stillWorks(self):
        r = FRB.IniResource("blend", "C:/mods/EiRemap", "EiBlend.buf")
        g = FRB.IniGroupedResource("group1", {"blend": r})

        self.assertFalse(g.isMissing({"blend"}))
        self.assertTrue(g.isMissing({"other"}))

    # ================================================
    # =================== deepcopy ====================

    def test_deepcopy_isolatesResourcesDict(self):
        g = FRB.IniGroupedResource("group1")
        resModObj = (0, "comp", "obj")
        g.resources[resModObj] = ("fileKey1", "rootSection", None, 0)

        g2 = copy.deepcopy(g)

        self.assertIsInstance(g2, FRB.IniGroupedResource)
        self.compareDict(g2.resources, g.resources)
        self.assertIsNot(g2.resources, g.resources)

        # mutate the copy -- original must be unaffected
        g2.resources[(1, "new", "obj")] = ("fileKey2", "rootSection2", None, 1)
        self.assertNotIn((1, "new", "obj"), g.resources)

        # mutate the original -- copy must be unaffected
        g.resources[resModObj] = ("changed", "rootSection", None, 0)
        self.assertEqual(g2.resources[resModObj][0], "fileKey1")


class RemapIniGroupedResourceTest(BaseUnitTest):
    """
    Tests for :class:`RemapIniGroupedResource` -- composes :class:`IniGroupedResource` (the same
    scratch-storage design above) with :class:`RemapIniResourceMixin`
    """

    def test_isInstanceOfIniGroupedResourceAndMixin(self):
        rg = FRB.RemapIniGroupedResource("group1")
        self.assertIsInstance(rg, FRB.IniGroupedResource)
        self.assertIsInstance(rg, FRB.RemapIniResourceMixin)

    def test_inheritsScratchStorage(self):
        rg = FRB.RemapIniGroupedResource("group1")
        resModObj = (0, "comp", "obj")
        rg.resources[resModObj] = ("fileKey1", "rootSection", None, 0)

        self.assertIn(resModObj, rg.resources)
        self.assertFalse(rg.isMissing({resModObj}))
        self.assertTrue(rg.isMissing({(9, "x", "y")}))

    def test_hasRequired_callableWithoutError(self):
        rg = FRB.RemapIniGroupedResource("group1")
        self.assertIsInstance(rg.hasRequired(), bool)

    def test_deepcopy_producesCorrectSubclass(self):
        rg = FRB.RemapIniGroupedResource("group1")
        rg.resources[(0, "comp", "obj")] = ("fileKey1", "rootSection", None, 0)

        rg2 = copy.deepcopy(rg)

        self.assertIsInstance(rg2, FRB.RemapIniGroupedResource)
        self.compareDict(rg2.resources, rg.resources)
        self.assertIsNot(rg2.resources, rg.resources)
