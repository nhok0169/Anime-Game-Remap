import sys
from typing import List, Optional, Callable

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


# A view that keeps everything it is told to display -- the shape a GUI or a backend server
#   forwarding messages to a frontend would take: subclass BaseLogger, implement write/read.
class ListLogger(FRB.BaseLogger):
    def __init__(self, *args, answers: Optional[List[str]] = None, **kwargs):
        super().__init__(*args, **kwargs)
        self.written: List[str] = []
        self.asked: List[str] = []
        self.answers = list(answers) if (answers is not None) else []

    def write(self, message: str):
        self.written.append(message)

    def read(self, desc: str) -> str:
        self.asked.append(desc)
        return self.answers.pop(0) if (self.answers) else ""


# A view that overrides a *higher-level* method, so C++ callers (openHeading, error, ...) must
#   reach the Python override through the trampoline rather than the base's own log().
class LogSpy(ListLogger):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.logged: List[str] = []

    def log(self, message: str):
        self.logged.append(message)
        super().log(message)


# A structured view: never renders heading text at all, records events instead.
class EventLogger(ListLogger):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.events = []

    def openHeading(self, txt: str, sideLen: int = 2, headingChar: str = "="):
        self.events.append(("open", txt, sideLen, headingChar))

    def closeHeading(self):
        self.events.append(("close",))

    def error(self, message: str):
        self.events.append(("error", message))

    def getStr(self, message: str) -> str:
        return f"[{self.prefix}] {message}"


class BaseLoggerTest(BaseUnitTest):
    ErrorHeader = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"

    def setUp(self):
        super().setUp()
        self._printed: List[str] = []
        self.patch("builtins.print", side_effect = lambda txt: self._printed.append(txt))

    # ========= abstract base ================================

    def test_bareBaseLogger_constructsButWriteIsAbstract(self):
        logger = FRB.BaseLogger(prefix = "p", logTxt = True, verbose = True)
        self.assertEqual(logger.prefix, "p")
        self.assertEqual(logger.logTxt, True)

        with self.assertRaises(RuntimeError):
            logger.write("x")

        with self.assertRaises(RuntimeError):
            logger.read("x")

        # log() reaches the abstract sink only when verbose
        with self.assertRaises(RuntimeError):
            logger.log("x")

    def test_bareBaseLoggerNotVerbose_formattingWorksWithoutASink(self):
        logger = FRB.BaseLogger(verbose = False, logTxt = True)
        logger.openHeading("h")
        logger.bulletPoint("b")
        logger.closeHeading()

        self.assertEqual(logger.loggedTxt, "#  --> == h ==\n#  --> - b\n#  --> =======\n")
        self.compareList(self._printed, [])

    def test_subclassNoOverrides_writeStillAbstract(self):
        class Empty(FRB.BaseLogger):
            pass

        logger = Empty()
        with self.assertRaises(RuntimeError):
            logger.log("x")

    def test_loggerIsBaseLogger_pythonSubclassIsNotLogger(self):
        self.assertIsInstance(FRB.Logger(), FRB.BaseLogger)
        self.assertIsInstance(ListLogger(), FRB.BaseLogger)
        self.assertNotIsInstance(ListLogger(), FRB.Logger)

    # ========================================================
    # ========= write/read overridden from Python ============

    def test_pythonSink_receivesRenderedLines(self):
        logger = ListLogger(prefix = "pre")
        logger.log("one")
        logger.includePrefix = False
        logger.log("two")
        logger.space()
        logger.bulletPoint("b")
        logger.list(["x", "y"], transform = lambda t: t.upper())

        self.compareList(logger.written, ["# pre --> one", "two", "", "- b", "1. X", "2. Y"])
        self.compareList(self._printed, [])

    def test_pythonSinkNotVerbose_nothingWritten(self):
        logger = ListLogger(verbose = False, logTxt = True)
        logger.log("one")
        self.compareList(logger.written, [])
        self.assertEqual(logger.loggedTxt, "#  --> one\n")

    def test_pythonSink_headingsRoundTrip(self):
        logger = ListLogger()
        logger.includePrefix = False

        logger.openHeading("outer", sideLen = 3, headingChar = "-")
        logger.openHeading("inner")
        self.compareList(logger.headings, [("outer", 3, "-"), ("inner", 2, "=")])

        logger.closeHeading()
        logger.closeHeading()
        logger.closeHeading()
        self.compareList(logger.headings, [])
        self.compareList(logger.written, ["--- outer ---", "== inner ==", "=" * (2 * (2 + 1) + len("inner")), "-" * (2 * (3 + 1) + len("outer"))])

    def test_pythonSink_boxAndError(self):
        logger = ListLogger()
        logger.includePrefix = False
        logger.verbose = False

        logger.box("a\nb", "#")
        self.compareList(logger.written, [])

        logger.error("bad\nthing")
        self.compareList(logger.written, ["", self.ErrorHeader, "bad", "thing", self.ErrorHeader, ""])
        self.assertEqual(logger.verbose, False)

    def test_pythonSource_inputAndWaitExit(self):
        logger = ListLogger(prefix = "q", logTxt = True, answers = ["42", ""])

        result = logger.input("meaning?")
        self.assertEqual(result, "42")
        self.compareList(logger.asked, ["# q --> meaning?"])

        logger.waitExit()
        self.compareList(logger.asked, ["# q --> meaning?", "\n== Press ENTER to exit =="])
        self.assertEqual(logger.includePrefix, True)
        self.assertEqual(logger.loggedTxt, "# q --> meaning?\nInput: 42\n\n== Press ENTER to exit ==\nInput: \n")

    def test_pythonSource_handleException(self):
        logger = ListLogger()
        logger.includePrefix = False

        try:
            raise KeyError("k")
        except KeyError as e:
            logger.handleException(e)

        self.assertEqual(logger.written[0], "")
        self.assertEqual(logger.written[1], self.ErrorHeader)
        self.assertEqual(logger.written[2], "")
        self.assertEqual(logger.written[3], "KeyError: 'k'")
        self.assertIn("Traceback (most recent call last)", "\n".join(logger.written))

    def test_pythonSubclassState_survivesRoundTripThroughModel(self):
        logger = ListLogger()
        logger.includePrefix = False
        model = FRB.Model(logger = logger)

        model.print("log", "via model")
        self.assertIs(model.logger, logger)
        self.compareList(model.logger.written, ["via model"])

    def test_inlineConstructedSubclass_noVariableHeld(self):
        # every argument constructed inline, nothing but the Model holding the view
        model = FRB.Model(logger = ListLogger(prefix = "inline"))
        model.print("log", "x")
        model.print("openHeading", "h")
        self.compareList(model.logger.written, ["# inline --> x", "# inline --> == h =="])

    # ========================================================
    # ========= higher-level overrides reached from C++ ======

    def test_logOverridden_cppCallersGoThroughIt(self):
        logger = LogSpy()
        logger.includePrefix = False

        # every one of these is a C++ method whose body calls the *virtual* log()
        logger.openHeading("h", sideLen = 1)
        logger.bulletPoint("b")
        logger.list(["l"])
        logger.box("m", "#")
        logger.split()
        logger.closeHeading()

        expected = ["= h =", "- b", "1. l", "#", "m", "#", "\n", "====="]
        self.compareList(logger.logged, expected)
        self.compareList(logger.written, expected)

    def test_logOverridden_errorTogglesVerboseAroundOverride(self):
        logger = LogSpy(verbose = False)
        logger.includePrefix = False

        logger.error("e")
        self.compareList(logger.logged, ["", self.ErrorHeader, "e", self.ErrorHeader, ""])
        self.compareList(logger.written, ["", self.ErrorHeader, "e", self.ErrorHeader, ""])
        self.assertEqual(logger.verbose, False)

    def test_structuredOverrides_noTextRendered(self):
        logger = EventLogger(prefix = "srv")

        logger.openHeading("Summary", sideLen = 10)
        logger.log("line")
        logger.closeHeading()
        logger.handleException("RemoteError", "msg", "tb")
        logger.handleException(ValueError("v"))

        self.compareList(logger.events[:3], [("open", "Summary", 10, "="), ("close",), ("error", "\nRemoteError: msg\n\ntb")])
        self.assertEqual(logger.events[3][0], "error")
        self.assertTrue(logger.events[3][1].startswith("\nValueError: v\n\n"))

        # getStr is virtual too: the C++ log() rendered through the Python override
        self.compareList(logger.written, ["[srv] line"])
        self.compareList(logger.headings, [])

    def test_superCallInsideOverride_reachesBaseNotRecursion(self):
        class Twice(ListLogger):
            def log(self, message: str):
                super().log(message)
                super().log(message)

        logger = Twice()
        logger.includePrefix = False
        logger.openHeading("h")
        self.compareList(logger.written, ["== h ==", "== h =="])

    def test_listOverride_receivesNoneForMissingTransform(self):
        seen = []

        class ListSpy(ListLogger):
            def list(self, lst: List[str], transform: Optional[Callable[[str], str]] = None):
                seen.append((list(lst), transform))
                super().list(lst, transform)

        logger = ListSpy()
        logger.includePrefix = False
        logger.list(["a"])
        logger.list(["b"], transform = str.upper)

        self.assertEqual(seen[0][0], ["a"])
        self.assertIsNone(seen[0][1])
        self.assertEqual(seen[1][0], ["b"])
        self.assertTrue(callable(seen[1][1]))
        self.compareList(logger.written, ["1. a", "1. B"])

    # ========================================================
    # ========= subclassing the console Logger ===============

    def test_loggerSubclassOverridesWrite_printNotUsed(self):
        class Captured(FRB.Logger):
            def __init__(self, *args, **kwargs):
                super().__init__(*args, **kwargs)
                self.lines = []

            def write(self, message: str):
                self.lines.append(message)

        logger = Captured(prefix = "c")
        logger.openHeading("h")
        logger.log("x")

        self.compareList(logger.lines, ["# c --> == h ==", "# c --> x"])
        self.compareList(self._printed, [])

        # read() is not overridden, so it still goes through builtins.input
        self.patch("builtins.input", side_effect = lambda desc: "typed")
        self.assertEqual(logger.input("?"), "typed")

    def test_loggerSubclassNoOverrides_printUsed(self):
        class Plain(FRB.Logger):
            pass

        logger = Plain()
        logger.includePrefix = False
        logger.log("x")
        self.compareList(self._printed, ["x"])

    # ========================================================
    # ========= defaults are not shared mutable state ========

    def test_defaultHeadingChar_notMutatedAcrossCalls(self):
        logger = ListLogger()
        logger.includePrefix = False
        logger.openHeading("a", headingChar = "*")
        logger.openHeading("b")
        self.compareList(logger.written, ["** a **", "== b =="])

    # ========================================================
