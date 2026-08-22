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

##### CppLocalImports
from ...core import IfTemplatePart
from ...core import ParseContext
##### EndCppLocalImports

##### LocalImports
from ...constants.GenericTypes import SympBooleanType, SymbolType
from ...constants.GlobalCompilerParts import GlobalCompilerParts
from ...constants.IniConsts import IniKeywords
from ...constants.GlobalPackageManager import GlobalPackageManager
from ...constants.Packages import PackageModules
from ...exceptions.SyntaxErr import SyntaxErr
from .IfPredLogicGenerator import IfPredLogicGenerator
from .SympyIfPredGenerator import SympyIfPredGenerator
from ...constants.IfPredPartType import IfPredPartType
##### EndLocalImports


##### Script
class IfPredPartOld(IfTemplatePart):
    """
    This class inherits from :class:`IfTemplatePart`

    .. deprecated::
        Superseded by the `Z3`_-based :class:`IfPredPart` (:attr:`query` here is a `sympy`_ query;
        :class:`IfPredPart`'s is a :class:`Z3Predicate`) -- kept only as a fallback for any
        remaining `sympy`_-typed call site outside the Ini Graph Editing subsystem (eg.
        :class:`IfPredLogicGenerator`/:class:`SympyIfPredGenerator`'s own tests). No real call site
        in this codebase constructs this class anymore as of :class:`ResGroupCollect`'s own
        migration onto the `Z3`_-based :class:`IfPredPart` :raw-html:`<br />` :raw-html:`<br />`

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

    query: Optional[Union[`sympy.Boolean`_, :class:`bool`]]
        The associated logical query for the predicate :raw-html:`<br />` :raw-html:`<br />`

        If this value is ``None``, then will parse the logical query from :attr:`str` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``None``

    id: Optional[:class:`int`]
        The id for the part. If this parameter is ``None``, will generate a new id for the part. :raw-html:`<br />` :raw-html:`<br />`

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

    def __init__(self, src: str, type: IfPredPartType, ctx: Optional[ParseContext] = None, vars: Optional[Dict[str, SymbolType]] = None, query: Optional[Union[SympBooleanType, bool]] = None, id: Optional[int] = None):
        super().__init__(id = id)

        self.src = src
        self.type = type
        self.query = True if (self.type == IfPredPartType.Else) else None

        if (self.type == IfPredPartType.EndIf or self.type == IfPredPartType.Else):
            return

        if (query is not None):
            self.query = query
            return

        testStr = self.getTestStr()
        if (ctx is None):
            ctx = ParseContext(testStr)
        else:
            ctx.lines = testStr.splitlines()

        if (vars is None):
            vars = {}

        self.query = self.getLogicQuery(ctx, vars)

    def clone(self, newId: bool = False) -> "IfPredPartOld":
        """
        Creates a copy of this part

        Parameters
        ----------
        newId: :class:`bool`
            Whether to generate a new id for the part :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``False``

        Returns
        -------
        :class:`IfPredPartOld`
            The cloned part
        """

        return type(self)(src = self.src, type = self.type, query = self.query, id = None if (newId) else self.id)

    def __copy__(self) -> "IfPredPartOld":
        """
        Creates a copy of this part (equivalent to :meth:`clone`); supports ``copy.copy()``

        .. note::
            Needed since :class:`IfTemplatePart` is a pybind11-bound class -- a fresh
            ``py::class_<...>`` registration doesn't support ``copy.copy()``/``copy.deepcopy()``
            for free, and that gap is inherited by any pure-Python subclass of it that doesn't
            override these itself
        """

        return self.clone()

    def __deepcopy__(self, memo: dict) -> "IfPredPartOld":
        """
        Creates a deep copy of this part (equivalent to :meth:`clone`); supports ``copy.deepcopy()``
        """

        return self.clone()

    def getTestStr(self) -> str:
        cleanedSrc = self.src.lstrip().lower()
        nonCapturingBeginSpaces = r"(?<=(^\s*))"

        regex = GlobalPackageManager.get(PackageModules.Regex.value)

        if (not self.type == IfPredPartType.Elif):
            result = regex.sub(nonCapturingBeginSpaces + self.type.value, "", self.src, flags=re.IGNORECASE, count = 1)
            result = regex.sub(IniKeywords.Then.value + r"(?=(\s*$))", "", result, flags=re.IGNORECASE, count = 1)
            return result

        if (cleanedSrc.startswith(IfPredPartType.Else.value)):
            result = regex.sub(nonCapturingBeginSpaces + IfPredPartType.Else.value, "", self.src, flags=re.IGNORECASE, count = 1)
            return regex.sub(nonCapturingBeginSpaces + IfPredPartType.If.value, "", result, flags=re.IGNORECASE, count = 1)

        return regex.sub(nonCapturingBeginSpaces + IfPredPartType.Elif.value, "", result, flags=re.IGNORECASE, count = 1)

    @classmethod
    def getLogicQuery(cls, ctx: ParseContext, vars: Dict[str, SymbolType]) -> Optional[Union[SympBooleanType, bool]]:
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

    @classmethod
    def getIfPredStr(cls, ctx: ParseContext) -> Optional[str]:
        """
        Generates the predicate used in the if ... else ... parts of a .ini file

        Parameters
        ----------
        ctx: :class:`ParseContext`
            The parsing context for reading the conditional predicate

        Returns
        -------
        Optional[:class:`str`]
            The generated predicate
        """

        result = None

        try:
            result = GlobalCompilerParts.SympyTokenizer.value.simplifiedMaximalMunch(ctx)
        except SyntaxErr as e:
            return None

        try:
            result = GlobalCompilerParts.SympyParser.value.parse(result, ctx = ctx)
        except SyntaxErr as e:
            return None

        try:
            result = SympyIfPredGenerator.generate(result)
        except Exception as e:
            return None

        return result

    def toStr(self, *args, linePrefix: Optional[str] = None, **kwargs) -> str:
        """
        Retrieves the part as a string

        Parameters
        ----------
        linePrefix: Optional[:class:`str`]
            The string that will prefix every line :raw-html:`<br />` :raw-html:`<br />`

            If this value is ``None``, then will use the original source specified at :attr:`src`
            Otherwise, the any left spacing from the original source will be stripped and the new prefix will be added

            **Default**: ``None``

        Returns
        -------
        :class:`str`
            The string representation of the part
        """

        result = f"{self.src}"
        if (linePrefix is not None):
            result = f"{linePrefix}{result.lstrip()}"

        if (not result):
            return result

        if (result[-1] == "\n"):
            result = result[:-1]
        return result
##### EndScript
