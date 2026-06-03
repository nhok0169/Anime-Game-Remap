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
from typing import List, Union, Optional, Set
from collections import deque
##### EndExtImports

##### LocalImports
from ...constants.GenericTypes import SympBooleanType
from ...tools.parsing.ParseTree import ParseTree
##### EndLocalImports


##### Script
# SympyIfPredData: Data related to an part of a predicate for a .ini file
class SympyIfPredData():
    def __init__(self, src: str, precedence: Optional[int] = None, reduction: Optional[str] = None):
        self.src = src
        self.precedence = precedence
        self.reduction = reduction


# SympyPrecendence: Data related to the precendence for a .ini file
class SympyPrecedence():
    def __init__(self, precedence: int, higherPrecedenceBracket: Optional[Set[str]] = None):
        self.precedence = precedence
        self.higherPrecedenceBracket = higherPrecedenceBracket if (higherPrecedenceBracket is not None) else set()


class SympyIfPredGenerator():
    """
    The conditional predicate generator for .ini files using a `sympy logic query`_

    eg.

    .. code-block:: ini
        :linenos:
        
        ~(($y$ | Ne($x$, $y$)) & (($x$ >= $y$) | ($x$ <= $y$)) & Eq($x$, $y$*$z$ - $y$ + $z$/3))
    """

    ComparePrecedence = SympyPrecedence(1, higherPrecedenceBracket = {"eq", "ne", "gt", "ge", "lt", "le", "eq func", "ne func", "gt func", "ge func", "lt func", "le func"})

    Precedence = {
            "and": SympyPrecedence(0, higherPrecedenceBracket = {"or", "or func"}),
            "or": SympyPrecedence(0, higherPrecedenceBracket = {"and", "and func"}),
            "and func": SympyPrecedence(0, higherPrecedenceBracket = {"or", "or func"}),
            "or func": SympyPrecedence(0, higherPrecedenceBracket = {"and", "and func"}),
            "eq": ComparePrecedence,
            "ne": ComparePrecedence,
            "gt": ComparePrecedence,
            "ge": ComparePrecedence,
            "lt": ComparePrecedence,
            "le": ComparePrecedence,
            "eq func": ComparePrecedence,
            "ne func": ComparePrecedence,
            "gt func": ComparePrecedence,
            "ge func": ComparePrecedence,
            "lt func": ComparePrecedence,
            "le func": ComparePrecedence,
            "add": SympyPrecedence(2),
            "subtract": SympyPrecedence(2),
            "multiply": SympyPrecedence(3),
            "divide": SympyPrecedence(3),
            "not": SympyPrecedence(4),
            "not func": SympyPrecedence(4),
    }

    @classmethod
    def generate(cls, parseTree: ParseTree) -> Optional[Union[SympBooleanType, bool]]:
        """
        Generates a logic query from the parse tree

        Parameters
        ----------
        parseTree: :class:`ParseTree`
            The tree to parse

        Returns
        -------
        Optional[Union[`sympy.Boolean`_, :class:`bool`]]
            The generated logic query
        """

        generationFuncs = {
            "start": lambda node, childrenPred, vars: childrenPred[1],
            "start empty": lambda node, childrenPred, vars: SympyIfPredData("0"),
            "pred reduce": lambda node, childrenPred, vars: childrenPred[0],
            "and": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "and", joinChar = " ", newReduction = "and"),
            "or": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "or", joinChar = " ", newReduction = "or"),
            "and func": lambda node, childrenPred, vars: cls.combineArgs(childrenPred[2], "and func", joinChar = " && ", newReduction = "and func"),
            "or func": lambda node, childrenPred, vars: cls.combineArgs(childrenPred[2], "or func", joinChar = " || ", newReduction = "or func"),
            "pred arg list": lambda node, childrenPred, vars: cls.addPredArg(childrenPred[0], childrenPred[2]),
            "pred args reduce": lambda node, childrenPred, vars: [childrenPred[0]],
            "test reduce": lambda node, childrenPred, vars: childrenPred[0],
            "eq": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "eq", joinChar = " ", newReduction = "eq"),
            "ne": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "ne", joinChar = " ", newReduction = "ne"),
            "gt": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "gt", joinChar = " ", newReduction = "gt"),
            "ge": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "ge", joinChar = " ", newReduction = "ge"),
            "lt": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "lt", joinChar = " ", newReduction = "lt"),
            "le": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "le", joinChar = " ", newReduction = "le"),
            "eq func": lambda node, childrenPred, vars: cls.combineArgs([childrenPred[2], childrenPred[4]], "eq func", joinChar = " == ", newReduction = "eq func"),
            "ne func": lambda node, childrenPred, vars: cls.combineArgs([childrenPred[2], childrenPred[4]], "ne func", joinChar = " != ", newReduction = "ne func"),
            "gt func": lambda node, childrenPred, vars: cls.combineArgs([childrenPred[2], childrenPred[4]], "gt func", joinChar = " > ", newReduction = "gt func"),
            "ge func": lambda node, childrenPred, vars: cls.combineArgs([childrenPred[2], childrenPred[4]], "ge func", joinChar = " >= ", newReduction = "ge func"),
            "lt func": lambda node, childrenPred, vars: cls.combineArgs([childrenPred[2], childrenPred[4]], "lt func", joinChar = " < ", newReduction = "lt func"),
            "le func": lambda node, childrenPred, vars: cls.combineArgs([childrenPred[2], childrenPred[4]], "le func", joinChar = " <= ", newReduction = "le func"),
            "keyexpr reduce": lambda node, childrenPred, vars: childrenPred[0],
            "true": lambda node, childrenPred, vars: childrenPred[0],
            "false": lambda node, childrenPred, vars: childrenPred[0],
            "addexpr reduce": lambda node, childrenPred, vars: childrenPred[0],
            "add": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "add", joinChar = " ", newReduction = "add"),
            "subtract": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "subtract", joinChar = " ", newReduction = "subtract"),
            "multexpr reduce": lambda node, childrenPred, vars: childrenPred[0],
            "multiply": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "multiply", joinChar = " ", newReduction = "multiply"),
            "divide": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "divide", joinChar = " ", newReduction = "divide"),
            "nterm reduce": lambda node, childrenPred, vars: childrenPred[0],
            "not": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "not", joinChar = "", newReduction = "not"),
            "not func": lambda node, childrenPred, vars: cls.combineArgs([SympyIfPredData("!"), childrenPred[2]], "not func", joinChar = "", newReduction = "not func"),
            "variable": lambda node, childrenPred, vars: childrenPred[0],
            "int": lambda node, childrenPred, vars: childrenPred[0],
            "float": lambda node, childrenPred, vars: childrenPred[0],
            "bracket loop": lambda node, childrenPred, vars: cls.combineArgs(childrenPred, "bracket loop", joinChar = "", newReduction = "bracket loop", newPrecedence = 0),

            "TRUE": lambda node, childrenPred, vars: SympyIfPredData("1"),
            "FALSE": lambda node, childrenPred, vars: SympyIfPredData("0"),
            "PLUS": lambda node, childrenPred, vars: SympyIfPredData("+"),
            "MINUS": lambda node, childrenPred, vars: SympyIfPredData("-"),
            "STAR": lambda node, childrenPred, vars: SympyIfPredData("*"),
            "SLASH": lambda node, childrenPred, vars: SympyIfPredData("/"),
            "LPAREN": lambda node, childrenPred, vars: SympyIfPredData("("),
            "RPAREN": lambda node, childrenPred, vars: SympyIfPredData(")"),
            "EQ": lambda node, childrenPred, vars: SympyIfPredData("=="),
            "NE": lambda node, childrenPred, vars: SympyIfPredData("!="),
            "LT": lambda node, childrenPred, vars: SympyIfPredData("<"),
            "GT": lambda node, childrenPred, vars: SympyIfPredData(">"),
            "LE": lambda node, childrenPred, vars: SympyIfPredData("<="),
            "GE": lambda node, childrenPred, vars: SympyIfPredData(">="),
            "AND": lambda node, childrenPred, vars: SympyIfPredData("&&"),
            "OR": lambda node, childrenPred, vars: SympyIfPredData("||"),
            "NOT": lambda node, childrenPred, vars: SympyIfPredData("!"),
            "EQFUNC": lambda node, childrenPred, vars: SympyIfPredData(""),
            "NEFUNC": lambda node, childrenPred, vars: SympyIfPredData(""),
            "LTFUNC": lambda node, childrenPred, vars: SympyIfPredData(""),
            "GTFUNC": lambda node, childrenPred, vars: SympyIfPredData(""),
            "LEFUNC": lambda node, childrenPred, vars: SympyIfPredData(""),
            "GEFUNC": lambda node, childrenPred, vars: SympyIfPredData(""),
            "ANDFUNC": lambda node, childrenPred, vars: SympyIfPredData(""),
            "ORFUNC": lambda node, childrenPred, vars: SympyIfPredData(""),
            "NOTFUNC": lambda node, childrenPred, vars: SympyIfPredData(""),
            "SPACE": lambda node, childrenPred, vars: SympyIfPredData(""),
            "TAB": lambda node, childrenPred, vars: SympyIfPredData(""),
            "ID": lambda node, childrenPred, vars: SympyIfPredData(node.token.val[:-1]),
            "INT": lambda node, childrenLobic, vars: SympyIfPredData(node.token.val),
            "FLOAT": lambda node, childrenPred, vars: SympyIfPredData(node.token.val)
        }

        exploreState = 0
        collectState = 1
        nodeIdStack = deque([(exploreState, parseTree.rootId)])
        logicStack = deque()

        while (nodeIdStack):
            state, nodeId = nodeIdStack.pop()

            if (state == exploreState):
                nodeIdStack.append((collectState, nodeId))
                childrenIds = parseTree.children.get(nodeId, [])

                for childId in childrenIds:
                    nodeIdStack.append((exploreState, childId))

                continue

            node = parseTree.getNode(nodeId)
            isChild = parseTree.isChild(nodeId)
            generateFunc = generationFuncs.get(node.token.type if (isChild) else node.prodId)

            if (generateFunc is None):
                generateFunc = lambda node, childrenPred, vars: None

            childrenIds = parseTree.children.get(nodeId, [])
            childrenPred = []
            for childId in childrenIds:
                childrenPred.append(logicStack.pop())

            nodeGeneration = generateFunc(node, childrenPred, vars)
            logicStack.append(nodeGeneration)

        result = logicStack.pop()
        return result.src
    
    @classmethod
    def addPredArg(cls, predArgs: List[SympyIfPredData], arg: str) -> List[SympyIfPredData]:
        predArgs.append(arg)
        return predArgs
    
    @classmethod
    def combineArgs(cls, args: List[SympyIfPredData], reduction: str, joinChar: str = "", newReduction: Optional[str] = None, newPrecedence: Optional[int] = None) -> SympyIfPredData:
        precedence = cls.Precedence.get(reduction)
        if (precedence is None):
            args = list(map(lambda arg: arg.src, args))
            return SympyIfPredData(joinChar.join(args), precedence = newPrecedence, reduction = newReduction)
        
        argsLen = len(args)
        for i in range(argsLen):
            arg = args[i]
            argPrecedence = cls.Precedence.get(arg.reduction)
            if (argPrecedence is None):
                continue

            if (precedence.precedence > arg.precedence or arg.reduction in precedence.higherPrecedenceBracket):
                arg.src = f"({arg.src})"

        if (newPrecedence is None):
            newPrecedence = precedence.precedence

        args = list(map(lambda arg: arg.src, args))
        return SympyIfPredData(joinChar.join(args), precedence = newPrecedence, reduction = newReduction)
##### EndScript
