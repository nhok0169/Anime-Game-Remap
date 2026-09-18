import sys
from unittest import mock
from typing import List

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


# Black-box tests for the C++-backed console view, FRB.Logger. Its output is captured by patching
#   builtins.print / builtins.input, which the binding deliberately routes through (see PyLogger.h)
#   -- so these tests double as the check that a mock of either builtin is honoured.
class LoggerTest(BaseUnitTest):
    ErrorHeader = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._logger = FRB.Logger()
        cls._printLines = []
        cls._inputLine = ""

    def print(self, txt: str):
        self._printLines.append(txt)

    def readSTDIN(self, description: str):
        self.print(description)
        return self._inputLine

    def patchPrint(self):
        self.patch("builtins.print", side_effect = lambda txt: self.print(txt))

    def setUp(self):
        super().setUp()
        self._logger = FRB.Logger()
        self._printLines = []
        self._inputLine = ""
        self.patch("builtins.input", side_effect = lambda desc: self.readSTDIN(desc))

    def compareLoggedLines(self, messages: List[str]) -> List[str]:
        result = []
        for msg in messages:
            result.append(self._logger.getStr(msg))

        self.compareList(result, self._printLines)
        return result

    # ========= constructor ==================================

    def test_defaultConstruction_defaultAtts(self):
        logger = FRB.Logger()
        self.assertEqual(logger.prefix, "")
        self.assertEqual(logger.logTxt, False)
        self.assertEqual(logger.verbose, True)
        self.assertEqual(logger.includePrefix, True)
        self.assertEqual(logger.loggedTxt, "")
        self.compareList(logger.headings, [])

    def test_keywordConstruction_attsSet(self):
        logger = FRB.Logger(prefix = "pre", logTxt = True, verbose = False)
        self.assertEqual(logger.prefix, "pre")
        self.assertEqual(logger.logTxt, True)
        self.assertEqual(logger.verbose, False)

        # the exact keyword shape remapService uses
        logger = FRB.Logger(logTxt = bool("some log path"), verbose = False)
        self.assertEqual(logger.logTxt, True)
        self.assertEqual(logger.verbose, False)

    def test_anyLogger_isBaseLogger(self):
        self.assertIsInstance(self._logger, FRB.BaseLogger)
        self.assertTrue(issubclass(FRB.Logger, FRB.BaseLogger))

    # ========================================================
    # ========= clear ========================================

    def test_loggedTxt_logTextCleared(self):
        self.patchPrint()
        self._logger.logTxt = True
        self._logger.log("Hello World")

        self.assertGreater(len(self._logger.loggedTxt), 0)
        self._logger.clear()
        self.assertEqual(self._logger.loggedTxt, "")

    # ========================================================
    # ========= default heading atts =========================

    def test_anyLogger_defaultHeadingAtts(self):
        self.assertEqual(self._logger.DefaultHeadingSideLen, 2)
        self.assertEqual(self._logger.DefaultHeadingChar, "=")
        self.assertEqual(FRB.Logger.DefaultHeadingSideLen, 2)
        self.assertEqual(FRB.Logger.DefaultHeadingChar, "=")
        self.assertEqual(FRB.Logger.ErrorHeader, self.ErrorHeader)

    # ========================================================
    # ========= loggedTxt ====================================

    def test_listOfTextDisableLog_emptyText(self):
        self._logger.includePrefix = False
        self._logger.verbose = False

        txtLst = ["hello", "", "\n", "25th baam\n"]
        for txt in txtLst:
            self._logger.log(txt)

        self.assertEqual(self._logger.loggedTxt, "")

    def test_listOfTextEnableLog_combinedTextSeperatedByNewLine(self):
        self._logger.includePrefix = False
        self._logger.verbose = False
        self._logger.logTxt = True

        txtLst = ["hello", "", "\n", "25th baam\n"]
        for txt in txtLst:
            self._logger.log(txt)

        self.assertEqual(self._logger.loggedTxt, "\n".join(txtLst) + "\n")

    # ========================================================
    # ========= getStr =======================================

    def test_normalPrefix_prefixWithMsg(self):
        self._logger.prefix = "some prefix"

        messages = ["message", "", "\t\n\t\n"]
        for msg in messages:
            result = self._logger.getStr(msg)
            self.assertEqual(result, f"# {self._logger.prefix} --> {msg}")

    def test_lineBreakedPrefix_lineBreakedPrefixWithMsg(self):
        self._logger.prefix = "\t\n"

        messages = ["message", "", "\t\n\t\n"]
        for msg in messages:
            result = self._logger.getStr(msg)
            self.assertEqual(result, f"# {self._logger.prefix} --> {msg}")

    def test_unicodePrefixAndMsg_roundTrips(self):
        self._logger.prefix = "接頭辞 🐱"
        self.assertEqual(self._logger.prefix, "接頭辞 🐱")
        self.assertEqual(self._logger.getStr("メッセージ"), "# 接頭辞 🐱 --> メッセージ")

    # ========================================================
    # ========= log ==========================================

    def test_notVerbose_nothingPrinted(self):
        self.patchPrint()

        self._logger.verbose = False
        self._logger.prefix = "some prefix"
        messages = ["message", "", "\t\n\t\n"]
        result = []
        for msg in messages:
            self._logger.log(msg)

        self.compareList(result, self._printLines)

    def test_verboseNotIncludePrefix_originalMessages(self):
        self.patchPrint()

        self._logger.includePrefix = False
        messages = ["message", "", "\t\n\t\n"]
        for msg in messages:
            self._logger.log(msg)

        self.compareList(self._printLines, messages)

    def test_verboseNotLogged_printedLinesWithoutLog(self):
        self.patchPrint()

        self._logger.prefix = "some prefix"
        messages = ["message", "", "\t\n\t\n"]

        for msg in messages:
            self._logger.log(msg)

        self.compareLoggedLines(messages)
        self.assertEqual(self._logger.loggedTxt, "")

    def test_verboseLogged_printedLinesWithLog(self):
        self.patchPrint()

        self._logger.prefix = "some prefix"
        self._logger.logTxt = True
        messages = ["message", "", "\t\n\t\n"]

        for msg in messages:
            self._logger.log(msg)

        result = self.compareLoggedLines(messages)
        result = '\n'.join(result) + "\n"
        self.assertEqual(self._logger.loggedTxt, result)

    def test_printPatched_printCalledWithOneArg(self):
        calls = []
        self.patch("builtins.print", side_effect = lambda *args, **kwargs: calls.append((args, kwargs)))
        self._logger.includePrefix = False
        self._logger.log("hi")
        self.compareList(calls, [(("hi",), {})])

    # ========================================================
    # ========= write / read =================================

    def test_write_printsExactly(self):
        self.patchPrint()
        self._logger.write("raw line")
        self._logger.write("")
        self.compareList(self._printLines, ["raw line", ""])
        self.assertEqual(self._logger.loggedTxt, "")

    def test_writeNotVerbose_stillPrints(self):
        self.patchPrint()
        self._logger.verbose = False
        self._logger.write("raw line")
        self.compareList(self._printLines, ["raw line"])

    def test_read_inputsExactly(self):
        self._inputLine = "answer"
        result = self._logger.read("question? ")
        self.compareList(self._printLines, ["question? "])
        self.assertEqual(result, "answer")

    # ========================================================
    # ========= split ========================================

    def test_noPrefix_noLog(self):
        self._logger.split()
        self.compareList([], self._printLines)

    def test_hasPrefix_splitWithLineBreak(self):
        self.patchPrint()

        self._logger.prefix = "some prefix"
        self._logger.log("Hello")
        self._logger.split()
        self._logger.log("Au revoir")

        self.compareLoggedLines(["Hello", "\n", "Au revoir"])

    def test_prefixResetAfterLog_noSplitUntilNextLog(self):
        self.patchPrint()
        self._logger.includePrefix = False

        self._logger.log("Hello")
        self._logger.prefix = "new prefix"
        self._logger.split()
        self.compareList(self._printLines, ["Hello"])  # nothing logged under the new prefix yet

        self._logger.log("again")
        self._logger.split()
        self.compareList(self._printLines, ["Hello", "again", "\n"])

    # ========================================================
    # ========= space ========================================

    def test_somePrefix_prefixWithoutAnySuffix(self):
        self.patchPrint()
        self._logger.space()
        self.compareLoggedLines([""])

    # ========================================================
    # ========= openHeading ==================================

    def test_defaultOpeningHeadingParams_defaultOpeningHeading(self):
        self.patchPrint()

        openHeadingTxt = "heading"
        self._logger.openHeading(openHeadingTxt)
        self.compareLoggedLines([f"== {openHeadingTxt} =="])
        self.assertEqual(len(self._logger.headings), 1)
        self.compareList(self._logger.headings, [(openHeadingTxt, 2, "=")])

    def test_multipleOpenHeadingCalls_manyOpenHeadingsWithMostRecentCallParams(self):
        self.patchPrint()

        finalSideLen = 3
        finalHeadingChar = "#"
        finalLoggedTxt = "combined"

        self._logger.openHeading("default")
        self._logger.openHeading("extended length", sideLen = 5)
        self._logger.openHeading("different side border", headingChar = "*")
        self._logger.openHeading("combined", sideLen = finalSideLen, headingChar = finalHeadingChar)

        self.compareLoggedLines(["== default ==", "===== extended length =====", "** different side border **", f"{finalSideLen * finalHeadingChar} {finalLoggedTxt} {finalSideLen * finalHeadingChar}"])
        self.assertEqual(len(self._logger.headings), 4)
        self.compareList(self._logger.headings, [("default", 2, "="), ("extended length", 5, "="), ("different side border", 2, "*"), ("combined", 3, "#")])

    def test_headingsProperty_copyNotView(self):
        self._logger.verbose = False
        self._logger.openHeading("a")

        headings = self._logger.headings
        headings.append(("b", 1, "-"))
        self.assertEqual(len(self._logger.headings), 1)

    def test_unicodeHeading_closingLineMatchesOpeningWidth(self):
        self.patchPrint()
        self._logger.includePrefix = False

        title = "見出し 🐱"
        self._logger.openHeading(title, sideLen = 3, headingChar = "-")
        self._logger.closeHeading()

        self.assertEqual(self._printLines[0], f"--- {title} ---")
        self.assertEqual(self._printLines[1], "-" * len(self._printLines[0]))

    def test_combiningAndZwjHeading_closingLineCountsGraphemes(self):
        self.patchPrint()
        self._logger.includePrefix = False

        # "e" + combining acute accent is 2 code points but 1 grapheme; the family emoji is a
        #   7-code-point ZWJ sequence rendering as 1 grapheme -- so the width is 3, not 10
        title = "é \U0001F468‍\U0001F469‍\U0001F467"
        self._logger.openHeading(title, sideLen = 1)
        self._logger.closeHeading()

        self.assertEqual(self._printLines[0], f"= {title} =")
        self.assertEqual(self._printLines[1], "=" * (2 * (1 + 1) + 3))

    # ========================================================
    # ========= closeHeading =================================

    def test_noOpeningHeading_noClosingHeading(self):
        self.patchPrint()
        self._logger.closeHeading()
        self.compareLoggedLines([])
        self.assertEqual(len(self._logger.headings), 0)

    def test_moreClosingHeadingThanOpeningHeading_wellFormedHeadings(self):
        self.patchPrint()

        self._logger.openHeading("test")
        self._logger.closeHeading()
        self._logger.openHeading("default")
        self._logger.openHeading("extended length", sideLen = 5)
        self._logger.closeHeading()
        self._logger.openHeading("different side border", headingChar = "*")
        self._logger.openHeading("combined", sideLen = 3, headingChar = "#")
        self._logger.closeHeading()
        self._logger.closeHeading()
        self._logger.closeHeading()
        self._logger.closeHeading()
        self._logger.closeHeading()
        self._logger.closeHeading()

        self.compareLoggedLines(["== test ==",
                                 "==========",
                                 "== default ==",
                                 "===== extended length =====",
                                 "===========================",
                                 "** different side border **",
                                 "### combined ###",
                                 "################",
                                 "***************************",
                                 "============="])
        self.assertEqual(len(self._logger.headings), 0)

    # ========================================================
    # ========= getBulletStr =================================

    def test_manyStrings_stringsPrefixedWithADash(self):
        self.assertEqual(self._logger.getBulletStr(""), "- ")
        self.assertEqual(self._logger.getBulletStr("\t\n"), "- \t\n")
        self.assertEqual(self._logger.getBulletStr("msg"), "- msg")
        self.assertEqual(FRB.Logger.getBulletStr("msg"), "- msg")

    # ========================================================
    # ========= getNumberedStr ===============================

    def test_manyStringsFirstPoint_stringsPrefixedWithNumberOne(self):
        num = 1
        self.assertEqual(self._logger.getNumberedStr("", num), f"{num}. ")
        self.assertEqual(self._logger.getNumberedStr("msg", num), f"{num}. msg")
        self.assertEqual(self._logger.getNumberedStr("\t\n\n\t", num), f"{num}. \t\n\n\t")

    def test_oneStringManyIntegers_stringPrefixedWithIntegers(self):
        msg = "msg"
        self.assertEqual(self._logger.getNumberedStr(msg, 8), f"8. {msg}")
        self.assertEqual(self._logger.getNumberedStr(msg, 9876), f"9876. {msg}")
        self.assertEqual(self._logger.getNumberedStr(msg, 0), f"0. {msg}")
        self.assertEqual(self._logger.getNumberedStr(msg, -0), f"0. {msg}")
        self.assertEqual(self._logger.getNumberedStr(msg, -9), f"-9. {msg}")
        self.assertEqual(self._logger.getNumberedStr(msg, -123), f"-123. {msg}")
        self.assertEqual(FRB.Logger.getNumberedStr(msg, 4), f"4. {msg}")

    # ========================================================
    # ========= bulletPoint ==================================

    def test_manyMessages_manyBulletPoints(self):
        self.patchPrint()

        self._logger.bulletPoint("one")
        self._logger.bulletPoint("two")
        self._logger.bulletPoint("three")

        self.compareLoggedLines(["- one", "- two", "- three"])

    # ========================================================
    # ========= list =========================================

    def test_manyMessagesNoTransform_orderedList(self):
        self.patchPrint()

        self._logger.list(["one", "two", "three"])
        self.compareLoggedLines(["1. one", "2. two", "3. three"])

    def test_manyMessagesPrefixTransform_orderedListAllMessagesPrefixed(self):
        self.patchPrint()

        self._logger.list(["one", "two", "three"], transform = lambda txt: f"negative {txt}")
        self.compareLoggedLines(["1. negative one", "2. negative two", "3. negative three"])

    def test_noMessages_noOrderedList(self):
        self.patchPrint()
        self._logger.list([])
        self.compareLoggedLines([])

    def test_nonCallableTransform_typeError(self):
        self.patchPrint()
        with self.assertRaises(TypeError):
            self._logger.list(["one"], transform = 3)

    # ========================================================
    # ========= box ==========================================

    def test_manyHeadersMsgWithoutNewLines_MessageSandwichedInHeader(self):
        self.patchPrint()

        msg = "bling bang \t bang born"
        headers = ["@@@@@", "\n\n\\yahallo\n\n", "", " ", "\n\n\n\n"]

        for header in headers:
            self._logger.box(msg, header)
            self.compareLoggedLines([header, msg, header])
            self._printLines = []

    def test_headerMsgWithNewLines_MessageSplitByNewLineAndSandwichedInHeader(self):
        self.patchPrint()

        header = "\n??????????????????????????\n"
        msg = "Butterfly Dream\n===============\n\nAm I the butterfly\nDreaming of being a man?\nOr am I the man\nDreaming of being a butterfly?"
        self._logger.box(msg, header)

        result = [header] +  msg.split("\n") + [header]
        self.compareLoggedLines(result)

    def test_msgWithEdgeNewLines_emptyLinesKeptLikePythonSplit(self):
        self.patchPrint()
        self._logger.includePrefix = False

        for msg in ["", "\n", "a\n", "\na", "a\n\nb"]:
            self._logger.box(msg, "#")
            self.compareList(self._printLines, ["#"] + msg.split("\n") + ["#"])
            self._printLines = []

    # ========================================================
    # ========= error ========================================

    def test_messageVerbose_MessageSandwichedInExclamationMarks(self):
        self.patchPrint()

        msg = "OH NO!\n\nThe end is nigh\nThis is the end..."
        self._logger.error(msg)

        result = ["", self.ErrorHeader] + msg.split("\n") + [self.ErrorHeader, ""]
        self.compareLoggedLines(result)

    def test_messageNotVerbose_MessageSandwichedInExclamationMarks(self):
        self.patchPrint()
        self._logger.verbose = False

        msg = "OH NO!\n\nThe end is nigh\nThis is the end..."
        self._logger.error(msg)
        self._logger.log("This will not be printed")

        result = ["", self.ErrorHeader] + msg.split("\n") + [self.ErrorHeader, ""]
        self.compareLoggedLines(result)
        self.assertEqual(self._logger.verbose, False)

    def test_messageNotVerboseLogged_onlyInLoggedTxt(self):
        self.patchPrint()
        self._logger.verbose = False
        self._logger.logTxt = True
        self._logger.includePrefix = False

        msg = "quiet error"
        self._logger.error(msg)

        self.compareList(self._printLines, [])
        self.assertEqual(self._logger.loggedTxt, "\n".join(["", self.ErrorHeader, msg, self.ErrorHeader, ""]) + "\n")

    # ========================================================
    # ========= handleException ==============================

    @mock.patch("traceback.format_exc")
    def test_someException_exceptionAndTraceback(self, m_format_exc):
        tracebackStr = "line 888: another error on another line..."
        exceptionMsg = "Nani!!"
        msg = f"\nNameError: {exceptionMsg}\n\n{tracebackStr}"

        self.patchPrint()
        m_format_exc.side_effect = lambda: tracebackStr

        self._logger.handleException(NameError(exceptionMsg))
        result = ["", self.ErrorHeader] + msg.split("\n") + [self.ErrorHeader, ""]
        self.compareLoggedLines(result)

    def test_liveException_realTraceback(self):
        self.patchPrint()
        self._logger.includePrefix = False

        try:
            raise ValueError("boom")
        except ValueError as e:
            self._logger.handleException(e)

        joined = "\n".join(self._printLines)
        self.assertIn("\nValueError: boom\n", joined)
        self.assertIn("Traceback (most recent call last)", joined)
        self.assertIn("raise ValueError", joined)

    def test_exceptionParts_sameShapeAsException(self):
        self.patchPrint()

        msg = "\nSomeRemoteError: it broke\n\nremote traceback"
        self._logger.handleException("SomeRemoteError", "it broke", "remote traceback")
        result = ["", self.ErrorHeader] + msg.split("\n") + [self.ErrorHeader, ""]
        self.compareLoggedLines(result)

    def test_exceptionPartsNoTraceback_emptyTraceback(self):
        self.patchPrint()

        msg = "\nSomeRemoteError: it broke\n\n"
        self._logger.handleException("SomeRemoteError", "it broke")
        result = ["", self.ErrorHeader] + msg.split("\n") + [self.ErrorHeader, ""]
        self.compareLoggedLines(result)

    # ========================================================
    # ========= input ========================================

    def test_noIncludePrefixEnableLogging_questionWithResponseWithoutPrefix(self):
        self._logger.includePrefix = False
        self._logger.logTxt = True

        msg = """
              Every abuser has been a victim to abuse.
              Every victim has killed their abuser.
              Every victim becomes an abuser.
              Every abuser is possessed by the spirit of their previous abuser.
              Then to the 'little sister' who is abusing me.
              Who are you really?
              """

        self._inputLine = "Ibitsu: Do you have a little sister?"
        result = self._logger.input(msg)

        self.compareList([msg], self._printLines)
        self.assertEqual(f"{msg}\nInput: {self._inputLine}\n", self._logger.loggedTxt)
        self.assertEqual(result, self._inputLine)

    def test_includePrefixNoLogging_questionWithReponseWithPrefix(self):
        msg = '''
              Statement: aLl cATs hAvE thE sAMe CuTENess

              pRooF bY iNDucTiON
              -=-==-----===-=-=-=
              let n be the number of cats

              BaSE CasE:
                for n = 1, we only have 1 cat, this cat has the same cuteness to themselves
                Therefore, the base case is true

              InDUCtiVE hYpOtHESIS:
                assume that for n = k for some k >= 1, all n cats have the same cuteness

              inDucTIve StEP:
                for k+1 cats, remove the first cat, we get that:
                    there are k cats and by the inductive hypothesis, the last k cats have the same cuteness

                from the k+1 cats, remove the last cat, we get that:
                    there are k cats and by the inductive hypothesis, the first k cats have the same cuteness

                From above, we get that the first cat has the same cuteness as the rest of the cats and the first cat
                    has the same cuteness as the last cat

                Therefore the inductive step is true
              '''

        self._inputLine = """
                          Ehhhh... Oni-chan... Neko-chan is cuter than the stray cat on the street nano desu 🐱
                          And your cute little sister is clearly smarter than my dumb baka oni-chan
                          """

        result = self._logger.input(msg)
        self.compareLoggedLines([msg])
        self.assertEqual(result, self._inputLine)
        self.assertEqual(self._logger.loggedTxt, "")

    def test_inputNotVerbose_stillAsks(self):
        self._logger.verbose = False
        self._logger.includePrefix = False
        self._inputLine = "yes"

        result = self._logger.input("continue?")
        self.compareList(self._printLines, ["continue?"])
        self.assertEqual(result, "yes")

    # ========================================================
    # ========= waitExit =====================================

    def test_includePrefixLogText_waitExitWithoutPrefix(self):
        self._logger.logTxt = True
        self._inputLine = "I totally pressed ENTER..."
        self._logger.waitExit()

        msg = "\n== Press ENTER to exit =="
        self.compareList([msg], self._printLines)
        self.assertEqual(f"{msg}\nInput: {self._inputLine}\n", self._logger.loggedTxt)
        self.assertEqual(self._logger.includePrefix, True)

    # ========================================================
    # ========= Model integration ============================

    def test_modelPrint_forwardsByName(self):
        self.patchPrint()

        model = FRB.Model(logger = self._logger)
        model.print("openHeading", "Summary", sideLen = 10)
        model.print("log", "done")
        model.print("space")
        model.print("closeHeading")

        self.compareLoggedLines(["========== Summary ==========", "done", "", "=" * (2 * (10 + 1) + len("Summary"))])

    # ========================================================
