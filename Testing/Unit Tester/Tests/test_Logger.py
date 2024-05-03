import sys
from unittest import mock
from .baseUnitTest import BaseUnitTest
from typing import List

sys.path.insert(1, '../../Fix-Raiden-Boss 2.0 (for all user )')
from src.FixRaidenBoss2 import FixRaidenBoss2 as FRB


class LoggerTest(BaseUnitTest):
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

    # ========= _setDefaultHeadingAtts =======================
        
    def test_anyLogger_defaultHeadingAtts(self):
        self._logger._setDefaultHeadingAtts()
        self.assertEqual(self._logger._headingTxtLen, 0)
        self.assertEqual(self._logger._headingSideLen, self._logger.DefaultHeadingSideLen)
        self.assertEqual(self._logger._headingChar, self._logger.DefaultHeadingChar)

    # ========================================================
    # ========= _addLogTxt ===================================
        
    def test_listOfTextDisableLog_emptyText(self):
        txtLst = ["hello", "", "\n", "25th baam\n"]
        for txt in txtLst:
            self._logger._addLogTxt(txt)

        self.assertEqual(self._logger._loggedTxt, "")

    def test_listOfTextEnableLog_combinedTextSeperatedByNewLine(self):
        self._logger.logTxt = True
        txtLst = ["hello", "", "\n", "25th baam\n"]
        for txt in txtLst:
            self._logger._addLogTxt(txt)

        self.assertEqual(self._logger._loggedTxt, "\n".join(txtLst) + "\n")

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
        self.assertEqual(self._logger._loggedTxt, "")

    def test_verboseLogged_printedLinesWithLog(self):
        self.patchPrint()

        self._logger.prefix = "some prefix"
        self._logger.logTxt = True
        messages = ["message", "", "\t\n\t\n"]

        for msg in messages:
            self._logger.log(msg)

        result = self.compareLoggedLines(messages)
        result = '\n'.join(result) + "\n"
        self.assertEqual(self._logger._loggedTxt, result)

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
        self.assertEqual(len(self._logger._headings), 1)

    def test_multipleOpenHeadingCalls_manyOpenHeadingsWithMostRecentCallParams(self):
        self.patchPrint()

        finalSideLen = 3
        finalHeadingChar = "#"
        finalLoggedTxt = "combined"

        self._logger.openHeading("default")
        self._logger.openHeading("extended length", sideLen = 5)
        self._logger.openHeading("different side border", headingChar = "*")
        self._logger.openHeading("combined", sideLen = "#", headingChar = 3)

        self.compareLoggedLines(["== default ==", "===== extended length =====", "** different side border **", f"{finalSideLen * finalHeadingChar} {finalLoggedTxt} {finalSideLen * finalHeadingChar}"])
        self.assertEqual(len(self._logger._headings), 4)
        
    # ========================================================
    # ========= closeHeading =================================
        
    def test_noOpeningHeading_noClosingHeading(self):
        self.patchPrint()
        self._logger.closeHeading()
        self.compareLoggedLines([])
        self.assertEqual(len(self._logger._headings), 0)

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
        self.assertEqual(len(self._logger._headings), 0)

    # ========================================================
    # ========= getBulletStr =================================
        
    def test_manyStrings_stringsPrefixedWithADash(self):
        self.assertEqual(self._logger.getBulletStr(""), "- ")
        self.assertEqual(self._logger.getBulletStr("\t\n"), "- \t\n")
        self.assertEqual(self._logger.getBulletStr("msg"), "- msg")

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
        
    # ========================================================
    # ========= box ==========================================
        
    def test_manyHeadersMsgWithoutNewLines_MessageSandwichedInHeader(self):
        self.patchPrint()

        msg = "bling bang \t bang born"
        headers = ["@@@@@", "\n\n\yahallo\n\n", "", " ", "\n\n\n\n"]

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

    # ========================================================
    # ========= error ========================================
        
    def test_messageVerbose_MessageSandwichedInExclamationMarks(self):
        self.patchPrint()

        header = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        msg = "OH NO!\n\nThe end is nigh\nThis is the end..."
        self._logger.error(msg)

        result = ["", header] + msg.split("\n") + [header, ""]
        self.compareLoggedLines(result)
        

    def test_messageNotVerbose_MessageSandwichedInExclamationMarks(self):
        self.patchPrint()
        self._logger.verbose = False

        header = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        msg = "OH NO!\n\nThe end is nigh\nThis is the end..."
        self._logger.error(msg)
        self._logger.log("This will not be printed")

        result = ["", header] + msg.split("\n") + [header, ""]
        self.compareLoggedLines(result)

    # ========================================================
    # ========= handleException ==============================
    
    @mock.patch("src.FixRaidenBoss2.FixRaidenBoss2.traceback.format_exc")
    def test_someException_exceptionAndTraceback(self, m_format_exc):
        tracebackStr = "line 888: another error on another line..."
        exceptionMsg = "Nani!!"
        header = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        msg = f"\nNameError: {exceptionMsg}\n\n{tracebackStr}"

        self.patchPrint()
        m_format_exc.side_effect = lambda: tracebackStr

        self._logger.handleException(NameError(exceptionMsg))
        result = ["", header] + msg.split("\n") + [header, ""]
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
        self.assertEqual(f"{msg}\nInput: {self._inputLine}\n", self._logger._loggedTxt)
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

    # ========================================================
    # ========= waitExit =====================================
        
    def test_includePrefixLogText_waitExitWithoutPrefix(self):
        self._logger.logTxt = True
        self._inputLine = "I totally pressed ENTER..."
        self._logger.waitExit()

        msg = "\n== Press ENTER to exit =="
        self.compareList([msg], self._printLines)
        self.assertEqual(f"{msg}\nInput: {self._inputLine}\n", self._logger._loggedTxt)
        
    # ========================================================
    
    

    