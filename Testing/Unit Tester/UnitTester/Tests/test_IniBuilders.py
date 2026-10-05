import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


_INI = ("[TextureOverrideRaidenBody]\nhash = 1a495487\nrun = CommandListRaidenBody\n"
        "\n[CommandListRaidenBody]\nvb0 = ResourceRaidenBody\n")


def _iniFile():
    FRB.CppGlobalModTypes.registerAll()
    return FRB.IniFile(None, _INI)


def _raiden():
    FRB.CppGlobalModTypes.registerAll()
    return next(modType for modType in FRB.CppGlobalModTypes.all() if modType.name == "Raiden")


class IniParseBuilderTest(BaseUnitTest):
    """
    Tests for :class:`IniParseBuilder`.

    The resolution logic (floor-matched version lookup, the fallback when a mod name has no row)
    is core-side behaviour. What these cover is the *binding*: that a Python callable can act as
    the factory, that the parser it returns comes back as **the same object** rather than a fresh
    wrapper of the base class, and that the version-dependent flavour is visible on a real mod
    type even though Python cannot construct one.
    """

    def test_pythonFactory_calledWithIniFileAndModTypeId(self):
        seen = []

        def factory(iniFile, modTypeId):
            seen.append((iniFile, modTypeId))
            return FRB.BaseIniParser(None)

        ini = _iniFile()
        FRB.IniParseBuilder(factory).build(ini, "Raiden", None, 7)

        self.assertEqual(len(seen), 1)
        self.assertIs(seen[0][0], ini)
        self.assertEqual(seen[0][1], 7)

    def test_pythonFactory_noModTypeId_passesNone(self):
        seen = []

        def factory(iniFile, modTypeId):
            seen.append(modTypeId)
            return FRB.BaseIniParser(None)

        FRB.IniParseBuilder(factory).build(_iniFile(), "Raiden")
        self.assertEqual(seen, [None])

    def test_pythonSubclassReturned_identityKept(self):
        # The whole point of holdPyStrategy: without it the Python half is freed the moment the
        # factory returns, and what comes back is a brand new plain BaseIniParser.
        class MyParser(FRB.BaseIniParser):
            pass

        made = MyParser(None)
        made.marker = "kept"

        built = FRB.IniParseBuilder(lambda iniFile, modTypeId: made).build(_iniFile(), "Raiden")

        self.assertIs(built, made)
        self.assertIsInstance(built, MyParser)
        self.assertEqual(built.marker, "kept")

    def test_defaultFactory_buildsCoreSideParser(self):
        built = FRB.IniParseBuilder().build(_iniFile(), "Raiden")

        # A core-built parser has no richer Python type to come back as -- CppBaseIniParser exists
        # precisely so it does not have to come back as None.
        self.assertIsInstance(built, FRB.CppBaseIniParser)
        self.assertNotIsInstance(built, FRB.BaseIniParser)

    def test_defaultFactory_isCallable(self):
        factory = FRB.IniParseBuilder.defaultFactory()
        self.assertIsInstance(factory(_iniFile(), None), FRB.CppBaseIniParser)

    def test_fixedFlavour_hasNoBuilderArgs(self):
        self.assertIsNone(FRB.IniParseBuilder().builderArgs)
        self.assertIsNone(FRB.IniParseBuilder(lambda iniFile, modTypeId: None).builderArgs)
        self.assertFalse(FRB.IniParseBuilder().errorOnNotFound)

    def test_modType_hasVersionDependentBuilder(self):
        builder = _raiden().iniParseBuilder

        self.assertIsInstance(builder, FRB.IniParseBuilder)
        self.assertIsInstance(builder.builderArgs, FRB.CppIniParseBuilderArgs)


class IniFixBuilderTest(BaseUnitTest):
    """
    Tests for :class:`IniFixBuilder`.

    Same shape as :class:`IniParseBuilderTest`, plus ``buildAll`` -- the entry point that
    actually matters, since one source mod may be fixed onto several targets.
    """

    def test_pythonFactory_fixedFlavour_calledWithEmptyTarget(self):
        # A fixed-factory builder ignores the mod names entirely -- it hands the factory an empty
        # target, matching the pure-Python original's "this argument has no effect if _buildCls is
        # not None" warning. The names only matter to the version-dependent flavour.
        seen = []
        parser = FRB.BaseIniParser(None)

        def factory(builtFor, toModName, modTypeId):
            seen.append((builtFor, toModName, modTypeId))
            return FRB.BaseIniFixer(None)

        FRB.IniFixBuilder(factory).build(parser, "Raiden", "RaidenShogun", modTypeId = 35)

        self.assertEqual(len(seen), 1)
        self.assertIs(seen[0][0], parser)
        self.assertEqual(seen[0][1], "")

        # The third argument is the ModTypeId of the mod type being fixed FROM. Unlike the mod
        # names it IS passed through on the fixed-factory path -- it is what lets the built fixer's
        # context resolve its own mod type, which nothing could do before it was threaded through.
        self.assertEqual(seen[0][2], 35)

    def test_pythonSubclassReturned_identityKept(self):
        class MyFixer(FRB.BaseIniFixer):
            pass

        made = MyFixer(None)
        built = FRB.IniFixBuilder(lambda parser, toModName, modTypeId: made).build(
            FRB.BaseIniParser(None), "Raiden", "RaidenShogun")

        self.assertIs(built, made)

    def test_buildAll_returnsTargetFixerPairs(self):
        made = FRB.BaseIniFixer(None)
        pairs = FRB.IniFixBuilder(lambda parser, toModName, modTypeId: made).buildAll(
            FRB.BaseIniParser(None), "Raiden")

        self.assertTrue(len(pairs) >= 1)
        for toModName, fixer in pairs:
            self.assertIsInstance(toModName, str)
            self.assertIs(fixer, made)

    def test_buildAll_fixedFlavour_ignoresFilter(self):
        # A fixed-factory builder has no targets to fan out over, so buildAll yields exactly one
        # entry under the empty name however it is filtered.
        builder = FRB.IniFixBuilder(lambda parser, toModName, modTypeId: FRB.BaseIniFixer(None))

        for filtered in (None, set(), {"RaidenShogun"}):
            pairs = builder.buildAll(FRB.BaseIniParser(None), "Raiden", None, None, filtered)
            self.assertEqual([name for name, _ in pairs], [""])

    def test_defaultFactory_buildsCoreSideFixer(self):
        built = FRB.IniFixBuilder().build(FRB.BaseIniParser(None), "Raiden", "RaidenShogun")

        self.assertIsInstance(built, FRB.CppBaseIniFixer)
        self.assertNotIsInstance(built, FRB.BaseIniFixer)

    def test_modType_hasVersionDependentBuilder(self):
        builder = _raiden().iniFixBuilder

        self.assertIsInstance(builder, FRB.IniFixBuilder)
        self.assertIsInstance(builder.builderArgs, FRB.CppIniFixBuilderArgs)


class IniRemoveBuilderTest(BaseUnitTest):
    """
    Tests for :class:`IniRemoveBuilder`.
    """

    def test_pythonFactory_calledWithIniFile(self):
        seen = []

        def factory(iniFile):
            seen.append(iniFile)
            return FRB.BaseIniRemover(None)

        ini = _iniFile()
        FRB.IniRemoveBuilder(factory).build(ini)

        self.assertEqual(len(seen), 1)
        self.assertIs(seen[0], ini)

    def test_pythonSubclassReturned_identityKept(self):
        class MyRemover(FRB.BaseIniRemover):
            pass

        made = MyRemover(None)
        built = FRB.IniRemoveBuilder(lambda iniFile: made).build(_iniFile())

        self.assertIs(built, made)

    def test_factoryReturningNone_fallsBackToDefault(self):
        # build() is contractually never None: a factory that hands back nothing is topped up with
        # the default one rather than having the null propagate to the caller.
        built = FRB.IniRemoveBuilder(lambda iniFile: None).build(_iniFile())

        self.assertIsNotNone(built)
        self.assertIsInstance(built, FRB.CppBaseIniRemover)

    def test_defaultFactory_buildsCoreSideRemover(self):
        built = FRB.IniRemoveBuilder().build(_iniFile())

        self.assertIsInstance(built, FRB.CppBaseIniRemover)
        self.assertNotIsInstance(built, FRB.BaseIniRemover)

    def test_modType_hasVersionDependentBuilder(self):
        builder = _raiden().iniRemoveBuilder

        self.assertIsInstance(builder, FRB.IniRemoveBuilder)
        self.assertIsInstance(builder.builderArgs, FRB.CppIniRemoveBuilderArgs)
