##### Credits

# ===== Anime Game Remap (AG Remap) =====
# Authors: Albert Gold#2696, NK#1321
#
# if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
# Special Thanks:
#   nguen#2011 (for support)
#   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
#   HazrateGolabi#1364 (for being awesome, and improving the code)

##### EndCredits


##### ExtImports
from collections import deque
import traceback
from typing import List, Optional, Callable
##### EndExtImports

##### LocalImports
from ..tools.Heading import Heading
##### EndLocalImports


##### Script
class Logger():
    """
    Class for pretty printing output to display on the console

    Parameters
    ----------
    prefix: :class:`str`
        line that is printed before any message is printed out :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ""

    logTxt: :class:`bool`
        Whether to log all the printed messages into a .txt file once the fix is done :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``False``

    verbose: :class:`bool`
        Whether to print out output :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    includePrefix: :class:`bool`
        Whether to include the prefix string when printing out a message

    verbose: :class:`bool`
        Whether to print out output

    logTxt: :class:`bool`
        Whether to log all the printed messages into a .txt file once the fix is done

    _prefix: :class:`str`
        line that is printed before any message is printed out

    _headings: Deque[:class:`Heading`]
        A stack of headings that have been opened (by calling :meth:`Heading.open`), but have not been closed yet (have not called :meth:`Heading.close` yet)

    _loggedTxt: :class:`str`
        The text that will be logged into a .txt file
    """

    DefaultHeadingSideLen = 2
    DefaultHeadingChar = "="

    def __init__(self, prefix: str = "", logTxt: bool = False, verbose: bool = True):
        self._prefix = prefix
        self.includePrefix = True
        self.verbose = verbose
        self.logTxt = logTxt
        self._loggedTxt = ""
        self._headings = deque()
        self._currentPrefixTxt = ""

        self._setDefaultHeadingAtts()

    @property
    def prefix(self):
        """
        The line of text that is printed before any message is printed out

        :getter: Returns such a prefix
        :setter: Sets up such a prefix for the logger
        :type: :class:`str`
        """
        return self._prefix
    
    @prefix.setter
    def prefix(self, newPrefix):
        self._prefix = newPrefix
        self._currentPrefixTxt = ""

    @property
    def loggedTxt(self):
        """
        The text to be logged into a .txt file

        :getter: Returns such a prefix
        :type: :class:`str`
        """
        return self._loggedTxt

    def clear(self):
        """
        Clears out any saved text from the logger
        """

        self._loggedTxt = ""

    def _setDefaultHeadingAtts(self):
        """
        Sets the default attributes for printing out a header line
        """

        self._headingTxtLen = 0
        self._headingSideLen = self.DefaultHeadingSideLen
        self._headingChar = self.DefaultHeadingChar

    def _addLogTxt(self, txt: str):
        """
        Appends the text to the logged output to be printed to a .txt file

        Parameters
        ----------
        txt: :class:`str`
            The text to be added onto the logged output
        """

        if (self.logTxt):
            self._loggedTxt += f"{txt}\n"

    def getStr(self, message: str):
        """
        Retrieves the string to be printed out by the logger

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out

        Returns
        -------
        :class:`str`
            The transformed text that the logger prints out
        """

        return f"# {self._prefix} --> {message}"

    def log(self, message: str):
        """
        Regularly prints text onto the console

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out
        """

        if (self.includePrefix):
            message = self.getStr(message)

        self._addLogTxt(message)
        self._currentPrefixTxt += f"{message}\n"

        if (self.verbose):
            print(message)

    def split(self):
        """
        Prints out a new line
        """

        if (self._currentPrefixTxt):
            self.log("\n")

    def space(self):
        """
        Prints out a space
        """
        self.log("")

    def openHeading(self, txt: str, sideLen: int = DefaultHeadingSideLen, headingChar = DefaultHeadingChar):
        """
        Prints out an opening heading

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out

        sideLen: :class:`int`
            How many characters we want for the side border of the heading :raw-html:`<br />`
            (see line 1 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`

            **Default**: 2

        headingChar: :class:`str`
            The type of character used to print the side border of the heading :raw-html:`<br />`
            (see line 3 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`

            **Default**: "="
        """

        heading = Heading(title = txt, sideLen = sideLen, sideChar = headingChar)
        self._headings.append(heading)
        self.log(heading.open())

    def closeHeading(self):
        """
        Prints out a closing heading that corresponds to a previous opening heading printed (see line 3 of the example at :class:`Heading`)
        """

        if (not self._headings):
            return

        heading = self._headings.pop()
        self.log(heading.close())

    @classmethod
    def getBulletStr(self, txt: str) -> str:
        """
        Creates the string for an item in an unordered list

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out

        Returns
        -------
        :class:`str`
            The text formatted as an item in an unordered list
        """
        return f"- {txt}"
    
    @classmethod
    def getNumberedStr(self, txt: str, num: int) -> str:
        """
        Creates the string for an ordered list

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out

        num: :class:`str`
            The number we want to print out before the text for the ordered list

        Returns
        -------
        :class:`str`
            The text formatted as an item in an ordered list
        """
        return f"{num}. {txt}"

    def bulletPoint(self, txt: str):
        """
        Prints out an item in an unordered list

        Parameters
        ----------
        txt: :class:`str`
            The message we want to print out
        """
        self.log(self.getBulletStr(txt))

    def list(self, lst: List[str], transform: Optional[Callable[[str], str]] = None):
        """
        Prints out an ordered list

        Parameters
        ----------
        lst: List[:class:`str`]
            The list of messages we want to print out

        transform: Optional[Callable[[:class:`str`], :class:`str`]]
            A function used to do any processing on each message in the list of messages

            If this parameter is ``None``, then the list of message will not go through any type of processing :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``
        """

        if (transform is None):
            transform = lambda txt: txt

        lstLen = len(lst)
        for i in range(lstLen):
            newTxt = transform(lst[i])
            self.log(self.getNumberedStr(newTxt, i + 1))

    def box(self, message: str, header: str):
        """
        Prints the message to be sandwiched by the text defined in the argument, ``header``

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out

        header: :class:`str`
            The string that we want to sandwich our message against
        """

        self.log(header)

        messageList = message.split("\n")
        for messagePart in messageList:
            self.log(messagePart)

        self.log(header)

    def error(self, message: str):
        """
        Prints an error message

        Parameters
        ----------
        message: :class:`str`
            The message we want to print out
        """

        prevVerbose = self.verbose
        if (not self.logTxt):
            self.verbose = True

        self.space()

        self.box(message, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        self.space()
        self.verbose = prevVerbose

    def handleException(self, exception: Exception):
        """
        Prints the message for an error

        Parameters
        ----------
        exception: :class:`Exception`
            The error we want to handle
        """

        message = f"\n{type(exception).__name__}: {exception}\n\n{traceback.format_exc()}"
        self.error(message)

    def input(self, desc: str) -> str:
        """
        Handles user input from the console

        Parameters
        ----------
        desc: :class:`str`
            The question/description being asked to the user for input

        Returns
        -------
        :class:`str`
            The resultant input the user entered
        """

        if (self.includePrefix):
            desc = self.getStr(desc)

        self._addLogTxt(desc)
        result = input(desc)
        self._addLogTxt(f"Input: {result}")

        return result

    def waitExit(self):
        """
        Prints the message used when the script finishes running
        """

        prevIncludePrefix = self.includePrefix
        self.includePrefix = False
        self.input("\n== Press ENTER to exit ==")
        self.includePrefix = prevIncludePrefix 
##### EndScripts