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

##### LocalImports
from .Error import Error
from ..tools.parsing.ParseContext import ParseContext
from ..tools.parsing.Token import Token
##### EndLocalImports


##### Script
class SyntaxErr(Error):
    """
    This Class inherits from :class:`Error`

    Exception when the syntax for some language is incorrect

    Parameters
    ----------
    ctx: :class:`ParseContext`
        The context for parsing some text

    token: :class:`Token`
        The token that caused the error

    process: :class:`str`
        The name of the process that caused the error :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``parsing``

    Attributes
    ----------
    ctx: :class:`ParseContext`
        The context for parsing some text

    token: :class:`Token`
        The token that caused the error
    """
    def __init__(self, ctx: ParseContext, token: Token, process: str = "parsing"):
        super().__init__(f'Invalid string, "{token.val}", found during {process}')
        self.ctx = ctx
        self.token = token

    def __str__(self) -> str:
        result = super().__str__()
        result += f"\n\n{self.getErrLocation()}"
        return result

    def getErrLocation(self) -> str:
        """
        Retrieves the location of the error
        """

        lineInd = self.token.lineNo - self.ctx.startLineNo
        linesLen = len(self.ctx.lines)
        if (lineInd >= linesLen):
            lineInd = linesLen - 1

        result = (f"Line No: {self.token.lineNo}\n"
                  f"Char No: {self.token.charNo}\n"
                  f"Line: {self.ctx.lines[lineInd]}")
        
        if (self.ctx.file is not None):
            result = f"File: {self.ctx.file}\n" + result

        return result
##### EndScript