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
from typing import  Dict, List, Union, Callable, Optional
from collections import deque
##### EndExtImports

##### LocalImports
from ...constants.GlobalPackageManager import GlobalPackageManager
from ...constants.Packages import PackageModules
from ...constants.GenericTypes import SymbolType, SympBooleanType
from ...tools.parsing.ParseTree import ParseTree
from ...tools.nodes.ParseNode import ParseNode
##### EndLocalImports


##### Script
class IfPredLogicGenerator():
    """
    The logical query generator used for conditional predicates within a .ini file

    eg.

    .. code-block:: ini
        :linenos:
        :emphasize-lines: 1,3

        if pred1
            ...
        else if pred2
            ...
        endif
    """

    @classmethod
    def generate(cls, parseTree: ParseTree, vars: Dict[str, SymbolType]) -> Optional[Union[SympBooleanType, bool]]:
        """
        Generates a logic query from the parse tree

        Parameters
        ----------
        parseTree: :class:`ParseTree`
            The tree to parse

        vars: Dict[:class:`str`, `sympy.Symbol`_]
            The variables to be saved from the parseTree :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the variables and the values are the variable instances

        Returns
        -------
        Optional[Union[`sympy.Boolean`_, :class:`bool`]]
            The generated logic query
        """

        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)
        generationFuncs = {
            "start": lambda node, childrenLogic, vars: childrenLogic[1],
            "start empty": lambda node, childrenLogic, vars: False,
            "pred reduce": lambda node, childrenLogic, vars: childrenLogic[0],
            "and": lambda node, childrenLogic, vars: cls.generateLogicBinOp(childrenLogic, lambda logic1, logic2: sympy.And(logic1, logic2)),
            "or": lambda node, childrenLogic, vars: cls.generateLogicBinOp(childrenLogic, lambda logic1, logic2: sympy.Or(logic1, logic2)),
            "ntest reduce": lambda node, childrenLogic, vars: childrenLogic[0],
            "not": lambda node, childrenLogic, vars: sympy.Not(childrenLogic[1]),
            "test reduce": lambda node, childrenLogic, vars: childrenLogic[0],
            "eq": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: sympy.Eq(logic1, logic2)),
            "ne": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: sympy.Ne(logic1, logic2)),
            "gt": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: sympy.Gt(logic1, logic2)),
            "ge": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: sympy.Ge(logic1, logic2)),
            "lt": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: sympy.Lt(logic1, logic2)),
            "le": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: sympy.Le(logic1, logic2)),
            "keyexpr reduce": lambda node, childrenLogic, vars: childrenLogic[0],
            "null": lambda node, childrenLogic, vars: 0,
            "addexpr reduce": lambda node, childrenLogic, vars: childrenLogic[0],
            "add": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: logic1 + logic2),
            "subtract": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: logic1 - logic2),
            "multexpr reduce": lambda node, childrenLogic, vars: childrenLogic[0],
            "multiply": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: logic1 * logic2),
            "divide": lambda node, childrenLogic, vars: cls.generateBinOp(childrenLogic, lambda logic1, logic2: logic1 / logic2),
            "variable": lambda node, childrenLogic, vars: childrenLogic[0],
            "int": lambda node, childrenLogic, vars: childrenLogic[0],
            "float": lambda node, childrenLogic, vars: childrenLogic[0],
            "bracket loop": lambda node, childrenLogic, vars: (childrenLogic[1]),

            "NULL": lambda node, childrenLogic, vars: 0,
            "PLUS": lambda node, childrenLogic, vars: None,
            "MINUS": lambda node, childrenLogic, vars: None,
            "STAR": lambda node, childrenLogic, vars: None,
            "SLASH": lambda node, childrenLogic, vars: None,
            "LPAREN": lambda node, childrenLogic, vars: None,
            "RPAREN": lambda node, childrenLogic, vars: None,
            "EQ": lambda node, childrenLogic, vars: None,
            "NE": lambda node, childrenLogic, vars: None,
            "LT": lambda node, childrenLogic, vars: None,
            "GT": lambda node, childrenLogic, vars: None,
            "LE": lambda node, childrenLogic, vars: None,
            "GE": lambda node, childrenLogic, vars: None,
            "AND": lambda node, childrenLogic, vars: None,
            "OR": lambda node, childrenLogic, vars: None,
            "NOT": lambda node, childrenLogic, vars: None,
            "SPACE": lambda node, childrenLogic, vars: None,
            "TAB": lambda node, childrenLogic, vars: None,
            "ID": lambda node, childrenLogic, vars: cls.generateVariable(node, vars),
            "INT": lambda node, childrenLobic, vars: int(node.token.val),
            "FLOAT": lambda node, childrenLogic, vars: float(node.token.val)
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
                generateFunc = lambda node, childrenLogic, vars: None

            childrenIds = parseTree.children.get(nodeId, [])
            childrenLogic = []
            for childId in childrenIds:
                childrenLogic.append(logicStack.pop())

            nodeGeneration = generateFunc(node, childrenLogic, vars)
            logicStack.append(nodeGeneration)

        result = logicStack.pop()

        if (type(result) == int or type(result) == float):
            result = sympy.Ne(result, 0)

        if (isinstance(result, sympy.logic.boolalg.BooleanTrue)):
            result = True
        elif (isinstance(result, sympy.logic.boolalg.BooleanFalse)):
            result = False

        return result

    @classmethod
    def generateVariable(cls, node: ParseNode, vars: Dict[str, SymbolType]) -> SymbolType:
        """
        Generates a variable

        Parameters
        ----------
        node: :class:`ParseNode`
            The node with the variable instantiation

        vars: Dict[:class:`str`, `sympy.Symbol`_]
            The variables to be saved from the parseTree :raw-html:`<br />` :raw-html:`<br />`

            The keys are the names of the variables and the values are the variable instances

        Returns
        -------
        `sympy.Symbol`_
            The corresponding variable
        """

        variableName = node.token.val.lower()
        result = vars.get(variableName)

        if (result is None):
            sympy = GlobalPackageManager.get(PackageModules.Sympy.value)
            result = sympy.Symbol(variableName[1:])
            vars[variableName] = result

        return result
    
    @classmethod
    def generateBinOp(cls, childrenLogic: List[Union[SympBooleanType, int, float, bool]], 
                      operator: Callable[[Union[SympBooleanType, int, float, bool], Union[SympBooleanType, int, float, bool]], Union[SympBooleanType, int, float, bool]]) -> Optional[Union[SympBooleanType, int, float, bool]]:
        """
        Generates the result for a binary operator

        Parameters
        ----------
        childrenLogic: List[Union[`sympy.Boolean`_, :class:`int`, :class:`float`, :class:`bool`]]
            The logic for the children nodes

        operator: Callable[[Union[`sympy.Boolean`_, :class:`int`, :class:`float`, :class:`bool`], Union[`sympy.Boolean`_, :class:`int`, :class:`float`, :class:`bool`]], Union[`sympy.Boolean`_, :class:`int`, :class:`float`, :class:`bool`]]
            The binary operator

        Returns
        -------
        Optional[Union[`sympy.Boolean`_, :class:`int`, :class:`float`, :class:`bool`]]
            The combined result
        """

        result = None
        childrenLen = len(childrenLogic)

        for i in range(childrenLen):
            childLogic = childrenLogic[i]
            if (childLogic is None):
                continue

            if (i == 0):
                result = childLogic
                continue

            result = operator(result, childLogic)

        return result
    
    @classmethod
    def generateLogicBinOp(cls, childrenLogic: List[Union[SympBooleanType, int, float, bool]], 
                           operator: Callable[[Union[SympBooleanType, float], Union[SympBooleanType, float]], SympBooleanType]) -> Optional[SympBooleanType]:
        """
        Generates the result for a logical binary operator

        Parameters
        ----------
        childrenLogic: List[Union[`sympy.Boolean`_, :class:`int`, :class:`float`, :class:`bool`]]
            The logic for the children nodes

        operator: Callable[[Union[`sympy.Boolean`_, :class:`bool`], Union[`sympy.Boolean`_, :class:`bool`]], `sympy.Boolean`_]
            The logical binary operator

        Returns
        -------
        Optional[`sympy.Boolean`_]
            The combined result
        """

        result = None
        childrenLen = len(childrenLogic)
        sympy = GlobalPackageManager.get(PackageModules.Sympy.value)

        for i in range(childrenLen):
            childLogic = childrenLogic[i]
            if (childLogic is None):
                continue

            if (not isinstance(childLogic, bool) and not isinstance(childLogic, sympy.logic.boolalg.Boolean)):
                childLogic = sympy.Ne(childLogic, 0)

            if (i == 0):
                result = childLogic
                continue

            result = operator(result, childLogic)

        return result
##### EndScript
