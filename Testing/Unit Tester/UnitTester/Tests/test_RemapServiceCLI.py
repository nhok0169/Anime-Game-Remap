import os
import sys
import shutil
import tempfile

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


# Tests for the BINDING surface of RemapServiceCLI, which is a different thing from the C++ class's
#   own behaviour. The model half is covered thoroughly by core/tests/RemapServiceCLI_test.cpp (25
#   tests) and core/tests/RemapService_fix_test.cpp; none of that is reachable from here, and none
#   of what is tested here is reachable from there. What this file covers is the seam:
#
#     * the string constructor's keyword names, which main.py passes by name -- a rename in
#       py::arg(...) breaks the CLI silently and no C++ test can see it
#     * that names/aliases/versions/download modes actually convert
#     * that a bad string surfaces as the REAL Python exception class, not a bare RuntimeError,
#       which is the whole point of the translation in PyRemapServiceCLI.cpp
#     * that a Python subclass's overrides are reached through the trampoline
#
# Note the deliberate absence of any "the fix produced the right output" assertion: every
#   IniFixer/IniParser is currently stubbed with its base class, so a real run generates no remapped
#   sections at all. Asserting on fix output today would bake the stub in as expected behaviour.
class RemapServiceCLITest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        # findByName only sees mod types that have been registered, and on a bare import the
        #   registry is empty -- it is otherwise populated as a side effect of the first classify().
        #   The C++ constructor calls registerMissing() itself; this mirrors it so the expected ids
        #   below can be looked up independently of that.
        FRB.CppGlobalModTypes.registerMissing()

    def setUp(self):
        super().setUp()
        self._folder = tempfile.mkdtemp(prefix="remapServiceCLITest")
        self.addCleanup(shutil.rmtree, self._folder, True)

    def makeCLI(self, **kwargs):
        kwargs.setdefault("path", self._folder)
        kwargs.setdefault("verbose", False)
        return FRB.RemapServiceCLI(**kwargs)

    # ------------------------------------------------------------------
    # The string constructor
    # ------------------------------------------------------------------

    def test_ctorKeywordsMatchWhatMainPasses(self):
        """
        Every keyword main.py names, passed at once. This is the test that catches a py::arg rename.
        """

        cli = FRB.RemapServiceCLI(path = self._folder, keepBackups = False, fixOnly = False,
                                  hideOrig = True, undoOnly = False, readAllInis = False,
                                  types = ["Raiden"], defaultType = None, forcedType = None,
                                  log = None, verbose = False, handleExceptions = True,
                                  remappedTypes = ["Ayaka"], version = "4.0", proxy = None,
                                  downloadMode = "disabled")

        self.assertFalse(cli.hasErrorsBeforeFix)
        self.assertFalse(cli.service.keepBackups)
        self.assertTrue(cli.service.hideOrig)
        self.assertTrue(cli.service.handleExceptions)

    def test_modTypeNamesBecomeIds(self):
        cli = self.makeCLI(types = ["Raiden"], remappedTypes = ["Ayaka"])

        self.assertFalse(cli.hasErrorsBeforeFix)
        self.assertEqual(cli.service.fromModTypeIds, {int(FRB.ModTypeId.Raiden)})
        self.assertEqual(cli.service.toModTypeIds, {int(FRB.ModTypeId.Ayaka)})

    def test_modTypeNamesIgnoreCaseAndWhitespace(self):
        """
        The --help text promises the names are not case sensitive, and a command line hands them
        over in whatever case the user typed.
        """

        cli = self.makeCLI(types = ["  rAiDeN  "])
        self.assertFalse(cli.hasErrorsBeforeFix)
        self.assertEqual(cli.service.fromModTypeIds, {int(FRB.ModTypeId.Raiden)})

        # An alias rather than a name, resolving to the mod type that owns it.
        byAlias = self.makeCLI(types = ["shogun"])
        self.assertFalse(byAlias.hasErrorsBeforeFix)
        self.assertEqual(byAlias.service.fromModTypeIds, {int(FRB.ModTypeId.Raiden)})

    def test_namingNoTypesMeansEveryType(self):
        """
        None and an empty list are the SAME answer -- a user who names no types wants all of them.
        This is the opposite of what an empty set means on RemapService.fromModTypeIds, and this
        constructor is where that ambiguity gets resolved.
        """

        self.assertIsNone(self.makeCLI().service.fromModTypeIds)
        self.assertIsNone(self.makeCLI(types = []).service.fromModTypeIds)

    def test_forcedTypeDecidesTheFixFromTypes(self):
        cli = self.makeCLI(types = ["Ayaka"], forcedType = "Raiden")

        self.assertEqual(cli.service.forcedModTypeIds, {int(FRB.ModTypeId.Raiden)})
        self.assertEqual(cli.service.fromModTypeIds, {int(FRB.ModTypeId.Raiden)},
                         "a forced type IS the fix-from answer, so what was named alongside it is ignored")

    def test_defaultTypeOnlyAppliesWithReadAllInis(self):
        self.assertEqual(len(self.makeCLI(defaultType = "Ayaka").service.defaultModTypeIds), 0)

        withAll = self.makeCLI(readAllInis = True, defaultType = "Ayaka")
        self.assertEqual(set(withAll.service.defaultModTypeIds), {int(FRB.ModTypeId.Ayaka)})

        # Unset with readAllInis on falls back to Raiden, the pure-Python original's own default.
        implied = self.makeCLI(readAllInis = True)
        self.assertEqual(set(implied.service.defaultModTypeIds), {int(FRB.ModTypeId.Raiden)})

    def test_versionAndDownloadModeConvert(self):
        cli = self.makeCLI(version = "4.0", downloadMode = "always")

        self.assertFalse(cli.hasErrorsBeforeFix)
        self.assertIsNotNone(cli.service.fromVersion)
        # The binding hands the mode back as the enum's string VALUE, not as the Python enum
        #   member -- core has its own DownloadMode enum and the two are mapped by value.
        self.assertEqual(cli.service.downloadMode, FRB.DownloadMode.Always.value)

    def test_unsetDownloadModeIsNormal(self):
        """
        The pure-Python original named a 'HardTexDriven' mode here that no longer exists on the enum,
        so its own unset case raised AttributeError. Normal is the replacement.
        """

        self.assertEqual(self.makeCLI().service.downloadMode, FRB.DownloadMode.Normal.value)

    # ------------------------------------------------------------------
    # Conversion failures
    # ------------------------------------------------------------------

    def test_aBadStringIsStoredNotRaised(self):
        cli = self.makeCLI(types = ["Raiden", "NotAModTypeAtAll"])

        self.assertTrue(cli.hasErrorsBeforeFix, "the constructor records rather than raises")
        self.assertEqual(cli.service.fromModTypeIds, {int(FRB.ModTypeId.Raiden)},
                         "and the names that did resolve are still collected")

    def test_conversionFailuresRaiseTheRealPythonExceptions(self):
        """
        The point of the translation in PyRemapServiceCLI.cpp: a caller catches
        FixRaidenBoss2.exceptions.InvalidModType, not a bare RuntimeError carrying a message.
        """

        with self.assertRaises(FRB.InvalidModType):
            self.makeCLI(types = ["NotAModTypeAtAll"]).raiseErrorsBeforeFix()

        with self.assertRaises(FRB.InvalidDownloadMode):
            self.makeCLI(downloadMode = "sometimes").raiseErrorsBeforeFix()

        # A plain ValueError, matching what the pure-Python original raised for a bad version.
        with self.assertRaises(ValueError):
            self.makeCLI(version = "not a version").raiseErrorsBeforeFix()

    def test_fixRaisesTheStoredErrorUnlessHandleExceptions(self):
        with self.assertRaises(FRB.InvalidModType):
            self.makeCLI(types = ["NotAModTypeAtAll"], handleExceptions = False).fix()

        # handleExceptions is the caller asking for it to be logged instead. Must not raise.
        self.makeCLI(types = ["NotAModTypeAtAll"], handleExceptions = True).fix()

    def test_conflictingOptionsIsRaisedFromPython(self):
        """
        fixOnly + undoOnly. This check lives in the PYTHON subclass because it names CommandOpts
        values; left to the C++ half the run would skip both halves and quietly report doing
        nothing.
        """

        with self.assertRaises(FRB.ConflictingOptions):
            self.makeCLI(fixOnly = True, undoOnly = True, handleExceptions = False).fix()

    # ------------------------------------------------------------------
    # The view
    # ------------------------------------------------------------------

    def test_verboseReadsAndWritesTheLoggersOwnFlag(self):
        cli = self.makeCLI(verbose = False)
        self.assertFalse(cli.verbose)

        cli.verbose = True
        self.assertTrue(cli.verbose)
        self.assertTrue(cli.logger.verbose)

        # The direction the pure-Python original got wrong: it kept a separate '_verbose' copy that
        #   only its own setter wrote, so assigning the logger's flag left the two disagreeing.
        cli.logger.verbose = False
        self.assertFalse(cli.verbose, "there is one answer, and it is the logger's")

    def test_logFolderBecomesAFilePath(self):
        logFolder = os.path.join(self._folder, "logs")
        cli = self.makeCLI(log = logFolder)

        self.assertIsNotNone(cli.log)
        self.assertEqual(os.path.basename(cli.log), FRB.FileTypes.Log.value,
                         "the file's name is always FileTypes.Log, never the caller's")
        self.assertTrue(cli.logger.logTxt)

        cli.log = None
        self.assertIsNone(cli.log)
        self.assertFalse(cli.logger.logTxt)

    def test_serviceSharesTheView(self):
        cli = self.makeCLI()
        self.assertIs(cli.service.logger, cli.logger)

    # ------------------------------------------------------------------
    # The trampoline
    # ------------------------------------------------------------------

    def test_pythonOverridesAreReachedThroughTheTrampoline(self):
        """
        Both hooks are called from fix(), never from the constructor -- a virtual call in a C++
        constructor does not dispatch to an override, so a subclass reshaping either would silently
        never run.
        """

        calls = []

        class RecordingCLI(FRB.RemapServiceCLI):
            def printModsToFix(self):
                calls.append("banner")
                super().printModsToFix()

            def addTips(self):
                calls.append("tips")

        cli = RecordingCLI(path = self._folder, verbose = False)
        self.assertEqual(calls, [], "the constructor calls neither")

        cli.fix()
        self.assertIn("banner", calls, "fix() reaches the subclass's banner")
        self.assertIn("tips", calls, "and its tips, since a clean run has no errors")

    def test_theShippedAddTipsNamesRealCommandOptions(self):
        """
        The Python subclass exists to own the option names; a tip quoting one that no longer exists
        would be worse than no tip.
        """

        cli = self.makeCLI(log = os.path.join(self._folder, "logs"))
        cli.addTips()

        logged = cli.logger.loggedTxt
        self.assertIn(FRB.CommandOpts.Revert.value, logged)
        self.assertIn(FRB.CommandOpts.All.value, logged)
