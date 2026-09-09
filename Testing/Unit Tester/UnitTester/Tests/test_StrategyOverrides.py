import os
import shutil
import sys
import tempfile
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


# A real Raiden .ini: the blend TextureOverride carries a hash the shipped hash table knows, which
# is what makes IniFile.classify recognise it. A made-up hash classifies as nothing and every
# assertion below would pass vacuously.
_RAIDEN_INI = ("; RaidenShogun\n"
               "\n"
               "[TextureOverrideRaidenShogunBlend]\n"
               "hash = 1a495487\n"
               "vb1 = ResourceRaidenShogunBlend\n"
               "handling = skip\n"
               "draw = 136416,0\n"
               "\n"
               "[ResourceRaidenShogunBlend]\n"
               "type = Buffer\n"
               "stride = 32\n"
               "filename = RaidenShogunBlend.buf\n")

# The mod object the blend graph is parsed into, and the one the resource graph is built into.
# They must differ: a resource edit whose target graph already exists takes the Ignore branch
# and never asks the context for the .ini file's sections at all.
_BLEND_OBJ = ("", "blend")
_RES_OBJ = (0, "", "blendRemapBlend")

# Relative to the Unit Tester's own working directory, the way its Paths.py does it.
_RAIDEN_BLEND = os.path.abspath(os.path.join("..", "..", "Data", "Mod Downloads", "GI",
                                             "Raiden", "4_0", "RaidenShogunBlend.buf"))

_RAIDEN = "Raiden"
_RAIDEN_TARGET = "RaidenBoss"


class _MarkedParser(FRB.BaseIniParser):
    """A parser carrying a label, so a test can tell WHICH factory built it.

    A subclass rather than an attribute set on a bare :class:`BaseIniParser`: a pybind11-bound class
    has no ``__dict__``, and only a plain Python subclass of it gets one.
    """

    def __init__(self, iniFile, marker):
        super().__init__(iniFile)
        self.marker = marker


class _MarkedFixer(FRB.BaseIniFixer):
    """The fixer half of :class:`_MarkedParser` -- same reason for being a subclass."""

    def __init__(self, parser, marker):
        super().__init__(parser)
        self.marker = marker


class _Boom(Exception):
    # A type the test owns, so an assertion on it cannot pass against some generic error the
    # machinery happened to raise on its own.
    pass


def _modType(name):
    FRB.CppGlobalModTypes.registerAll()
    return next(modType for modType in FRB.CppGlobalModTypes.all() if modType.name == name)


def _version(text):
    return FRB.CppVersion.parse(text)


class StrategyOverridesTest(BaseUnitTest):
    """
    Tests for :class:`CppStrategyOverrides`.

    The table is **process-wide** and deliberately unsynchronised, so every test here clears it on
    the way in and on the way out --- a leaked override would otherwise change the behaviour of
    whatever test module happens to run next, which is a far worse failure than a red test.

    Two things these pin that were each got wrong once:

    * **Version matching floor-matches**, mirroring :class:`ModDictAssets`, rather than matching
      exactly. A real run resolves a mod's version off the ``.ini`` file and normally passes *no*
      version at all, so exact matching made an override registered for ``6.1`` --- the literal
      case "override Raiden 6.1" means --- fire zero times.
    * **A Python-built parser has to work against the core** :class:`IniFile`. That combination
      was unreachable while the pure-Python ``IniFile`` existed and is the normal case now, and it
      is the only thing these overrides are for.
    """

    def setUp(self):
        super().setUp()
        FRB.CppStrategyOverrides.clear()
        self.addCleanup(FRB.CppStrategyOverrides.clear)

    # -------------------------------------------------------------------------------------
    # helpers
    # -------------------------------------------------------------------------------------
    def _iniFile(self):
        FRB.CppGlobalModTypes.registerAll()
        return FRB.IniFile(None, _RAIDEN_INI)

    def _markedParserFactory(self, marker, calls=None):
        def factory(iniFile, modTypeId):
            if (calls is not None):
                calls.append((iniFile, modTypeId))

            return _MarkedParser(iniFile, marker)

        return factory

    def _markedFixerFactory(self, marker, calls=None):
        def factory(parser, toModName, modTypeId):
            if (calls is not None):
                calls.append((parser, toModName, modTypeId))

            return _MarkedFixer(parser, marker)

        return factory

    def _buildParser(self, version=None):
        return _modType(_RAIDEN).iniParseBuilder.build(self._iniFile(), _RAIDEN, version)

    def _boomParserFactory(self):
        def factory(iniFile, modTypeId):
            raise _Boom("deliberate failure")

        return factory

    def _markerOf(self, built):
        return getattr(built, "marker", None)

    # -------------------------------------------------------------------------------------
    # empty / clear
    # -------------------------------------------------------------------------------------
    def test_empty_nothingRegistered_isTrue(self):
        self.assertTrue(FRB.CppStrategyOverrides.empty())

    def test_empty_parserRegistered_isFalse(self):
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("p"))
        self.assertFalse(FRB.CppStrategyOverrides.empty())

    def test_empty_fixerRegistered_isFalse(self):
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, _RAIDEN_TARGET, self._markedFixerFactory("f"))
        self.assertFalse(FRB.CppStrategyOverrides.empty())

    def test_clear_removesBothKinds(self):
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("p"))
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, _RAIDEN_TARGET, self._markedFixerFactory("f"))

        FRB.CppStrategyOverrides.clear()

        self.assertTrue(FRB.CppStrategyOverrides.empty())
        self.assertIsNone(self._markerOf(self._buildParser()))

    # -------------------------------------------------------------------------------------
    # setParser
    # -------------------------------------------------------------------------------------
    def test_setParser_takesPrecedenceOverTheBuiltInRow(self):
        # Raiden has a real compiled-in parser factory, so this is a genuine override rather than
        # filling a hole.
        self.assertIsNone(self._markerOf(self._buildParser()))

        calls = []
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("mine", calls))

        built = self._buildParser()

        self.assertEqual(self._markerOf(built), "mine")
        self.assertEqual(len(calls), 1)

    def test_setParser_factoryGetsTheIniFileAndModTypeId(self):
        calls = []
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("mine", calls))

        ini = self._iniFile()
        _modType(_RAIDEN).iniParseBuilder.build(ini, _RAIDEN, None, 35)

        self.assertEqual(len(calls), 1)
        self.assertIs(calls[0][0], ini)
        self.assertEqual(calls[0][1], 35)

    def test_setParser_pythonSubclassIdentityKept(self):
        class MyParser(FRB.BaseIniParser):
            pass

        made = MyParser(None)
        FRB.CppStrategyOverrides.setParser(_RAIDEN, lambda iniFile, modTypeId: made)

        built = self._buildParser()

        self.assertIs(built, made)
        self.assertIsInstance(built, MyParser)

    def test_setParser_wrongModName_doesNotFire(self):
        FRB.CppStrategyOverrides.setParser("NotARealModName", self._markedParserFactory("mine"))

        self.assertIsNone(self._markerOf(self._buildParser()))

    # -------------------------------------------------------------------------------------
    # version resolution -- the part that was got wrong first
    # -------------------------------------------------------------------------------------
    def test_setParser_versionedOverride_firesWhenNoVersionIsAsked(self):
        # THE case this class exists for. A run resolves a mod's version off the .ini file and
        # normally has none to pass, so "no version asked" has to mean "the newest override", not
        # "only a versionless override". Exact matching was tried here and fired zero times on an
        # ordinary run.
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("v61"), version="6.1")

        self.assertEqual(self._markerOf(self._buildParser()), "v61")

    def test_setParser_versionedOverride_floorMatchesAHigherVersion(self):
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("v61"), version="6.1")

        self.assertEqual(self._markerOf(self._buildParser(_version("7.0"))), "v61")

    def test_setParser_versionedOverride_doesNotApplyBelowItself(self):
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("v61"), version="6.1")

        self.assertIsNone(self._markerOf(self._buildParser(_version("5.0"))))

    def test_setParser_severalVersions_highestAtOrBelowWins(self):
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("v40"), version="4.0")
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("v61"), version="6.1")

        self.assertEqual(self._markerOf(self._buildParser(_version("5.0"))), "v40")
        self.assertEqual(self._markerOf(self._buildParser(_version("6.1"))), "v61")
        self.assertEqual(self._markerOf(self._buildParser(_version("9.9"))), "v61")

        # No version asked means the newest registered.
        self.assertEqual(self._markerOf(self._buildParser()), "v61")

    def test_setParser_versionlessOverride_isTheFallback(self):
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("any"))
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("v61"), version="6.1")

        # A versioned entry applies -> it wins.
        self.assertEqual(self._markerOf(self._buildParser(_version("6.1"))), "v61")

        # None applies (asked below every versioned entry) -> the versionless one is used.
        self.assertEqual(self._markerOf(self._buildParser(_version("2.0"))), "any")

    def test_setParser_versionlessOverride_aloneAppliesToEveryVersion(self):
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("any"))

        for version in (None, _version("1.0"), _version("6.1"), _version("99.0")):
            self.assertEqual(self._markerOf(self._buildParser(version)), "any")

    def test_setParser_sameVersionTwice_replacesRatherThanShadows(self):
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("first"), version="6.1")
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("second"), version="6.1")

        self.assertEqual(self._markerOf(self._buildParser()), "second")

    # -------------------------------------------------------------------------------------
    # setFixer
    # -------------------------------------------------------------------------------------
    def _buildAllFixers(self, fromModName=_RAIDEN, fromVersion=None, filteredToModNames=None):
        return _modType(_RAIDEN).iniFixBuilder.buildAll(FRB.BaseIniParser(None), fromModName,
                                                        fromVersion, None, filteredToModNames)

    def test_setFixer_takesPrecedenceOverTheBuiltInRow(self):
        before = self._buildAllFixers()
        self.assertEqual([name for name, _ in before], [_RAIDEN_TARGET])
        self.assertIsNone(self._markerOf(before[0][1]))

        calls = []
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, _RAIDEN_TARGET,
                                          self._markedFixerFactory("mine", calls))

        after = self._buildAllFixers()

        # Replaced, not appended: the target still appears exactly once.
        self.assertEqual([name for name, _ in after], [_RAIDEN_TARGET])
        self.assertEqual(self._markerOf(after[0][1]), "mine")
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][1], _RAIDEN_TARGET)

    def test_setFixer_newTarget_reachesBuildAll(self):
        # The point of fixerTargets(): a target the compiled-in table has no row for at all is what
        # lets a brand-new remap be prototyped, rather than only an existing one replaced.
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, "SomeBrandNewTarget",
                                          self._markedFixerFactory("brandNew"))

        pairs = self._buildAllFixers()
        byName = {name: fixer for name, fixer in pairs}

        self.assertIn("SomeBrandNewTarget", byName)
        self.assertEqual(self._markerOf(byName["SomeBrandNewTarget"]), "brandNew")

        # The built-in target is still built, by the built-in row.
        self.assertIn(_RAIDEN_TARGET, byName)
        self.assertIsNone(self._markerOf(byName[_RAIDEN_TARGET]))

    def test_setFixer_newTarget_respectsTheTargetFilter(self):
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, "SomeBrandNewTarget",
                                          self._markedFixerFactory("brandNew"))

        pairs = self._buildAllFixers(filteredToModNames={_RAIDEN_TARGET})

        self.assertEqual([name for name, _ in pairs], [_RAIDEN_TARGET])

    def test_setFixer_wrongToModName_doesNotFire(self):
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, "SomeBrandNewTarget",
                                          self._markedFixerFactory("brandNew"))

        byName = {name: fixer for name, fixer in self._buildAllFixers()}

        self.assertIsNone(self._markerOf(byName[_RAIDEN_TARGET]))

    def test_setFixer_wrongFromModName_doesNotFire(self):
        FRB.CppStrategyOverrides.setFixer("NotARealModName", _RAIDEN_TARGET,
                                          self._markedFixerFactory("mine"))

        byName = {name: fixer for name, fixer in self._buildAllFixers()}

        self.assertEqual(sorted(byName), [_RAIDEN_TARGET])
        self.assertIsNone(self._markerOf(byName[_RAIDEN_TARGET]))

    def test_setFixer_versionedOverride_firesWhenNoVersionIsAsked(self):
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, _RAIDEN_TARGET,
                                          self._markedFixerFactory("v61"), version="6.1")

        byName = {name: fixer for name, fixer in self._buildAllFixers()}

        self.assertEqual(self._markerOf(byName[_RAIDEN_TARGET]), "v61")

    def test_setFixer_versionedOverride_doesNotApplyBelowItself(self):
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, _RAIDEN_TARGET,
                                          self._markedFixerFactory("v61"), version="6.1")

        byName = {name: fixer for name, fixer in self._buildAllFixers(fromVersion=_version("5.0"))}

        self.assertIsNone(self._markerOf(byName[_RAIDEN_TARGET]))

    def test_setFixer_pythonSubclassIdentityKept(self):
        class MyFixer(FRB.BaseIniFixer):
            pass

        made = MyFixer(None)
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, _RAIDEN_TARGET,
                                          lambda parser, toModName, modTypeId: made)

        byName = {name: fixer for name, fixer in self._buildAllFixers()}

        self.assertIs(byName[_RAIDEN_TARGET], made)

    # -------------------------------------------------------------------------------------
    # the whole point: a Python strategy driving a real run over the core IniFile
    # -------------------------------------------------------------------------------------
    def test_pythonParserOverride_parsesTheCoreIniFile(self):
        # The third strategy-context case: a strategy built from Python, running against a bound
        # core IniFile. Everything in the Py* contexts that reads a pure-Python IniFile attribute
        # has to delegate instead, and the parse result has to survive being converted back --
        # PyGIMIParser.collectParseResult cast the graphs to an unregistered C++ type and threw.
        seen = []
        blend = ("", "blend")

        def classify(parser, sectionName, section, disjoint, part, kvps):
            if (sectionName.lower().startswith("textureoverride")
                    and "blend" in sectionName.lower()):
                seen.append(sectionName)
                return [blend]
            return []

        FRB.CppStrategyOverrides.setParser(
            _RAIDEN,
            lambda iniFile, modTypeId: FRB.GIMIParser(iniFile, modObjs=[blend],
                                                      objTargetFuncs=[classify]))

        ini = self._iniFile()

        # Completing is the assertion, not politeness: the collector that turns this parser's
        # Python graphs back into C++ ones cast them to a type pybind never registered, so this
        # call raised "Unable to cast Python instance of type IniSectionGraph to C++ type '?'" for
        # any parse that classified anything at all. IniFile.parse itself hands nothing back to
        # Python by design -- what it built is pinned by the fix test below.
        ini.parse()

        self.assertEqual(seen, ["TextureOverrideRaidenShogunBlend"])

    def test_pythonParserOverride_producesARealFix(self):
        # The acceptance criterion for the whole feature, at unit size: a parser written in Python
        # and registered here, feeding the COMPILED-IN Raiden fixer, has to produce a correct
        # remapped section -- not merely run.
        #
        # Two separate things this pins, both of which failed silently rather than loudly:
        #
        #  * a fixer takes ITS .ini file from parser.getIniFile(), and a Python-built parser left
        #    that null, so the fix came back keyed "0" and nothing reached disk
        #  * with it null, the block was rendered against no file at all -- so asserting on the
        #    block's CONTENT, not just its key, is what makes this test worth having
        blend = ("", "blend")

        def classify(parser, sectionName, section, disjoint, part, kvps):
            if (sectionName.lower().startswith("textureoverride")
                    and "blend" in sectionName.lower()):
                return [blend]
            return []

        FRB.CppStrategyOverrides.setParser(
            _RAIDEN,
            lambda iniFile, modTypeId: FRB.GIMIParser(iniFile, modObjs=[blend],
                                                      objTargetFuncs=[classify]))

        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "RaidenShogun.ini")
            with open(path, "w", encoding="utf-8") as handle:
                handle.write(_RAIDEN_INI)

            FRB.CppGlobalModTypes.registerAll()
            ini = FRB.IniFile(path)
            blocks = ini.fix(keepBackup=False)

            self.assertEqual(sorted(blocks), [path])

            fixed = blocks[path]
            self.assertIn("[TextureOverrideRaidenShogunRaidenBossRemapBlend]", fixed)
            self.assertIn("[ResourceRaidenShogunRaidenBossRemapBlend]", fixed)
            self.assertIn("filename = RaidenShogunRaidenBossRemapBlend.buf", fixed)

            # The boss's own blend hash, not the source mod's -- the remap really happened rather
            # than the section merely being copied under a new name.
            self.assertIn("hash = fe5c0180", fixed)
            self.assertIn("hash = 1a495487", fixed)  # the original section is still there

    def test_pythonFixerOverride_fixesTheCoreIniFile(self):
        # The fixer half of the third context case. PyIniFixContext builds its delegate to the core
        # context in its CONSTRUCTOR, but a PyGIMIFixer constructs that context with None and then
        # assigns the .ini file straight onto the member afterwards -- so every delegation in it was
        # dead, and this raised ``'IniFile' object has no attribute 'filePath'`` inside the
        # per-.ini guard, where nothing printed it.
        blend = ("", "blend")
        built = []

        def classify(parser, sectionName, section, disjoint, part, kvps):
            if (sectionName.lower().startswith("textureoverride")
                    and "blend" in sectionName.lower()):
                return [blend]
            return []

        def makeFixer(parser, toModName, modTypeId):
            fixer = FRB.GIMIFixer(parser)
            built.append(toModName)
            return fixer

        # modTypeId passed through on purpose. A Python fixer takes its own mod type from its
        # parser's ``modTypeId``, so a factory that drops the argument it was handed leaves the
        # fix unable to name the mod it is for -- the block comes out under the generic "GI Remap"
        # heading instead of "Raiden Remap". Nothing errors; the output is just quietly wrong.
        FRB.CppStrategyOverrides.setParser(
            _RAIDEN,
            lambda iniFile, modTypeId: FRB.GIMIParser(iniFile, modObjs=[blend],
                                                      objTargetFuncs=[classify],
                                                      modTypeId=modTypeId))
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, _RAIDEN_TARGET, makeFixer)

        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "RaidenShogun.ini")
            with open(path, "w", encoding="utf-8") as handle:
                handle.write(_RAIDEN_INI)

            FRB.CppGlobalModTypes.registerAll()
            ini = FRB.IniFile(path)
            blocks = ini.fix(keepBackup=False)

        self.assertEqual(built, [_RAIDEN_TARGET])
        self.assertEqual(sorted(blocks), [path])
        self.assertIn("Raiden Remap", blocks[path])

    def test_pythonParserOverride_isNotUsedForOtherModTypes(self):
        # An override is keyed by mod name, so a differently-classified .ini file must still get
        # its own compiled-in parser.
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._markedParserFactory("mine"))

        amber = _modType("Amber")
        built = amber.iniParseBuilder.build(self._iniFile(), "Amber")

        self.assertIsNone(self._markerOf(built))

    # -------------------------------------------------------------------------------------
    # stats
    # -------------------------------------------------------------------------------------
    def _runService(self, folder):
        # A whole run rather than IniFile.fix, because stats only exists on the service and the
        # per-.ini guard that fills in ``skipped`` only runs there.
        FRB.CppGlobalModTypes.registerAll()
        path = os.path.join(folder, "RaidenShogun.ini")
        with open(path, "w", encoding="utf-8") as handle:
            handle.write(_RAIDEN_INI)

        service = FRB.RemapService(path = folder, keepBackups = False)
        service.fix()
        return (service, path)

    def test_stats_cleanRun_countsTheFixedIni(self):
        # The baseline the failing runs below are read against. Asserting on the file's CONTENT as
        # well as the count is the point: "counted as fixed" and "actually rewritten" are separate
        # claims here, and this repo has shipped the first without the second before.
        with tempfile.TemporaryDirectory() as folder:
            service, path = self._runService(folder)
            stats = service.stats

            self.assertEqual(sorted(stats.ini.fixed), [path])
            self.assertEqual(dict(stats.ini.skipped), {})

            with open(path, "r", encoding = "utf-8") as handle:
                written = handle.read()

            self.assertIn("[TextureOverrideRaidenShogunRaidenBossRemapBlend]", written)

    def test_stats_parserThatRaises_recordsTheSkippedIni(self):
        # No logger is attached, so nothing is printed anywhere -- stats is the ONLY way an
        # embedding caller learns the file failed. That is the case the property exists for.
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._boomParserFactory())

        with tempfile.TemporaryDirectory() as folder:
            service, path = self._runService(folder)
            stats = service.stats

            self.assertEqual(list(stats.ini.fixed), [])
            self.assertEqual(sorted(stats.ini.skipped), [path])

    def test_stats_skippedKeepsTheOriginalPythonException(self):
        # Not a stringified stand-in: the guard holds an ``exception_ptr``, and rethrowing it gives
        # back a ``py::error_already_set`` still carrying the object Python raised. A caller can
        # therefore catch on its own exception type rather than parsing a message.
        FRB.CppStrategyOverrides.setParser(_RAIDEN, self._boomParserFactory())

        with tempfile.TemporaryDirectory() as folder:
            service, path = self._runService(folder)
            error = service.stats.ini.skipped[path]

            self.assertIsInstance(error, _Boom)
            self.assertEqual(str(error), "deliberate failure")

    def test_stats_isASnapshotRatherThanAView(self):
        # PyRemapStats is a standalone class, not a binding of AGRemapCore::RemapStats, so every
        # access converts afresh. Pinned because the two obvious reader mistakes -- holding one
        # object across a second fix, and editing it expecting the service to notice -- both look
        # reasonable and both silently do nothing.
        with tempfile.TemporaryDirectory() as folder:
            service, path = self._runService(folder)

            self.assertIsNot(service.stats, service.stats)

            taken = service.stats
            taken.ini.fixed.clear()
            self.assertEqual(sorted(service.stats.ini.fixed), [path])

    # -------------------------------------------------------------------------------------
    # the resource-edit seam
    # -------------------------------------------------------------------------------------
    def _blendClassifier(self):
        def classify(parser, sectionName, section, disjoint, part, kvps):
            low = sectionName.lower()
            if (low.startswith("textureoverride") and "blend" in low):
                return [_BLEND_OBJ]
            return []

        return classify

    def _sectionText(self, graph, sectionName):
        return graph.sections[sectionName].toStr()

    def test_pythonResourceEdit_buildsAgainstTheCoreIniFile(self):
        # The resource-edit half of the context seam, which was the last one still reading
        # attributes of the pure-Python IniFile deleted on 2026-09-03. Every accessor on
        # PyIniResEditContext is exercised at once here, because a resource edit needs all of them:
        #
        #   sectionIfTemplates()  the resource graph below is built FROM the .ini file's own parsed
        #                         sections -- "type = Buffer" could come from nowhere else
        #   z3Ctx()               handed to createGraph alongside them
        #   iniFolder()           the two absolute paths on the built model
        #   storeResource()       the model reaching ini.getResources() at all
        #
        # The first two are why this raised AttributeError before; the fourth is why it then did
        # nothing after they were fixed. Note what a weaker assertion would have missed: the
        # RENAMING half of the edit succeeded through both of those bugs, so a test that checked
        # only the .ini text would have passed against a fix that builds no resource at all.
        FRB.CppGlobalModTypes.registerAll()
        modType = _modType(_RAIDEN)

        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "RaidenShogun.ini")
            with open(path, "w", encoding = "utf-8") as handle:
                handle.write(_RAIDEN_INI)

            ini = FRB.IniFile(path)
            ini.classify()

            parser = FRB.GIMIParser(ini, modObjs = [_BLEND_OBJ],
                                    objTargetFuncs = [self._blendClassifier()])
            groups = parser.parse()

            replace = FRB.RemapBlendReplace(_RES_OBJ, resType = "blend")
            collect = FRB.ResRegCollect({(0, "", "blend"): "vb1"}, {"blend": replace})
            collect.editFromIni(groups, ini, modType, _RAIDEN_TARGET)

            graphs = groups[0].graphs
            self.assertIn(("", "blendRemapBlend"), graphs)

            # Cloned from the core .ini file's own parse -- sectionIfTemplates() delegating.
            resSection = self._sectionText(graphs[("", "blendRemapBlend")],
                                           "ResourceRaidenShogunRaidenBossRemapBlend")
            self.assertIn("type = Buffer", resSection)
            self.assertIn("stride = 32", resSection)

            # The reference was repointed at it.
            blendSection = self._sectionText(graphs[("", "blend")],
                                             "TextureOverrideRaidenShogunBlend")
            self.assertIn("vb1 = ResourceRaidenShogunRaidenBossRemapBlend", blendSection)

            resources = list(ini.getResources())
            self.assertEqual(len(resources), 1)

            resource = resources[0]
            self.assertEqual(resource.srcPath, os.path.join(folder, "RaidenShogunBlend.buf"))
            self.assertEqual(resource.fixedPath,
                             os.path.join(folder, "RaidenShogunRaidenBossRemapBlend.buf"))

            # Not None is the assertion: the vertex group remap is looked up off the mod type, and
            # a collector that does not forward modType to its resource edits leaves buildResModel
            # returning None -- no model, no vgRemap, and a .ini file pointing at a .buf that
            # nobody builds.
            self.assertIsNotNone(resource.vgRemap)

    def test_pythonResourceEdit_writesARemappedBlend(self):
        # The same edit through the real service over a real Blend.buf, because "a model was built"
        # and "the mod works" are different claims and only the second one matters.
        #
        # Every piece of the fix here is written in Python -- parser, fixer, collector and resource
        # edit -- and installed through the overrides. That is the whole point of the feature, and
        # this is the only test that runs it end to end.
        if (not os.path.isfile(_RAIDEN_BLEND)):
            self.skipTest("the Raiden download asset is not in this checkout")

        FRB.CppGlobalModTypes.registerAll()
        classify = self._blendClassifier()

        def makeParser(iniFile, modTypeId):
            return FRB.GIMIParser(iniFile, modObjs = [_BLEND_OBJ], objTargetFuncs = [classify],
                                  modTypeId = modTypeId)

        def makeFixer(parser, toModName, modTypeId):
            # resType "blend" rather than the default "resourceRemapBlend": RemapStats.get keys its
            # buckets by the first spelling, so the other one fixes the file and counts nothing.
            replace = FRB.RemapBlendReplace(_RES_OBJ, resType = "blend")
            collect = FRB.ResRegCollect({(0, "", "blend"): "vb1"}, {"blend": replace})

            # modsToFix is not optional here even though the signature allows it: GIMIFixer runs its
            # graph group edits once per mod it is fixing TO, so an empty list runs them zero times
            # and the fix comes out as a plain copy of the original sections.
            return FRB.GIMIFixer(parser, graphGroupEdits = [collect], modsToFix = [toModName])

        FRB.CppStrategyOverrides.setParser(_RAIDEN, makeParser)
        FRB.CppStrategyOverrides.setFixer(_RAIDEN, _RAIDEN_TARGET, makeFixer)

        with tempfile.TemporaryDirectory() as folder:
            with open(os.path.join(folder, "RaidenShogun.ini"), "w", encoding = "utf-8") as handle:
                handle.write(_RAIDEN_INI)

            srcPath = os.path.join(folder, "RaidenShogunBlend.buf")
            shutil.copyfile(_RAIDEN_BLEND, srcPath)

            service = FRB.RemapService(path = folder, keepBackups = False)
            service.fix()

            fixedPath = os.path.join(folder, "RaidenShogunRaidenBossRemapBlend.buf")
            self.assertTrue(os.path.isfile(fixedPath))

            with open(srcPath, "rb") as handle:
                srcBytes = handle.read()
            with open(fixedPath, "rb") as handle:
                fixedBytes = handle.read()

            # Same length, different content. A remap rewrites the vertex group indices in place, so
            # a file that came out byte-identical would mean the remap silently did nothing, and one
            # of a different length would mean it wrote something that is not a blend at all.
            self.assertEqual(len(fixedBytes), len(srcBytes))
            self.assertNotEqual(fixedBytes, srcBytes)

            # The FIXED path, not the source: a blend is recorded under the file that was
            # written.
            self.assertEqual(sorted(service.stats.blend.fixed), [fixedPath])
            self.assertEqual(dict(service.stats.blend.skipped), {})

            with open(os.path.join(folder, "RaidenShogun.ini"), "r", encoding = "utf-8") as handle:
                written = handle.read()

            self.assertIn("[ResourceRaidenShogunRaidenBossRemapBlend]", written)
            self.assertIn("vb1 = ResourceRaidenShogunRaidenBossRemapBlend", written)
