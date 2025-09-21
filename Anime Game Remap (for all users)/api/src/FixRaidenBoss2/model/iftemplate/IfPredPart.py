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
import re
from typing import Optional, Union, Dict
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import SympBooleanType, SymbolType
from ...constants.GlobalCompilerParts import GlobalCompilerParts
from ...exceptions.SyntaxErr import SyntaxErr
from ...tools.parsing.ParseContext import ParseContext
from .IfTemplatePart import IfTemplatePart
from .IfPredLogicGenerator import IfPredLogicGenerator
from ...constants.IfPredPartType import IfPredPartType
##### EndLocalImports


##### Script
class IfPredPart(IfTemplatePart):
    """
    This class inherits from :class:`IfTemplatePart`

    Class for defining the predicate part of an :class:`IfTemplate`

    .. note::
        see :class:`IfTemplate` for more details

    Parameters
    ----------
    src: :class:`str`
        The original string within the :class:`IfTemplate`

    type: :class:`IfPredPartType`
        The type of predicate encountered

    ctx: Optional[:class:`ParseContext`]
        The context for parsing the predicate :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    vars: Optional[Dict[:class:`str`, `sympy.Symbol`_]]
        The variables to save for the corresponding .ini file :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    Attributes
    ----------
    src: :class:`str`
        The original string within the :class:`IfTemplate` 

    type: :class:`IfPredPartType`
        The type of predicate encountered

    query: Optional[Union[`sympy.Boolean`_, :class:`bool`]]
        The associated logical query to the predicate
    """

    def __init__(self, src: str, type: IfPredPartType, ctx: Optional[ParseContext] = None, vars: Optional[Dict[str, SymbolType]] = None):
        self.src = src
        self.type = type
        self.query = True if (self.type == IfPredPartType.Else) else None

        if (self.type == IfPredPartType.EndIf or self.type == IfPredPartType.Else):
            return

        testStr = self.getTestStr()
        if (ctx is None):
            ctx = ParseContext(testStr)
        else:
            ctx.lines = testStr.splitlines()

        if (vars is None):
            vars = {}

        self.query = self.getLogicQuery(ctx, vars)

    def getTestStr(self) -> str:
        if (not self.type == IfPredPartType.Elif):
            return re.sub(self.type.value, "", self.src, flags=re.IGNORECASE, count = 1)
        
        cleanedSrc = self.src.lstrip().lower()
        if (cleanedSrc.startswith(IfPredPartType.Else.value)):
            result = re.sub(IfPredPartType.Else.value, "", self.src, flags=re.IGNORECASE, count = 1)
            return re.sub(IfPredPartType.If.value, "", result, flags=re.IGNORECASE, count = 1)
        
        return re.sub(IfPredPartType.Elif.value, "", result, flags=re.IGNORECASE, count = 1)
    
    def getLogicQuery(self, ctx: ParseContext, vars: Dict[str, SymbolType]) -> Optional[Union[SympBooleanType, bool]]:
        """
        Generates the corresponding `sympy`_ logical query

        Parameters
        ----------
        ctx: :class:`ParseContext`
            The parsing context for reading the conditional predicate

        vars: Dict[:class:`str`, `sympy.Symbol`_]
            The variables to be saved for the .ini file :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the variables and the values are the variable instances

        Returns
        -------
        Optional[Union[`sympy.Boolean`_, :class:`bool`]]
            The generated logic query
        """

        result = None

        try:
            result = GlobalCompilerParts.IfPredTokenizer.value.simplifiedMaximalMunch(ctx)
        except SyntaxErr as e:
            return None
        
        try:
            result = GlobalCompilerParts.IfPredParser.value.parse(result, ctx = ctx)
        except SyntaxErr as e:
            return None
        
        try:
            result = IfPredLogicGenerator.generate(result, vars)
        except Exception as e:
            return None
        
        return result

    def toStr(self) -> str:
        return f"{self.src}"
##### EndScript