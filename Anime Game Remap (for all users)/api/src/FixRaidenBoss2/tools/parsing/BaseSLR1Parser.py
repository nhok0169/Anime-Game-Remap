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
import uuid
from collections import defaultdict, deque
from typing import Dict, List, Set, Tuple, Hashable, Union, Optional, DefaultDict
##### EndExtImports

##### LocalImports
from ..ListTools import ListTools
from ...exceptions.SyntaxErr import SyntaxErr
from ..DFA import DFA
from ..nodes.ParseNode import ParseNode
from .ParseTree import ParseTree
from .Token import Token
from .ParseContext import ParseContext
##### EndLocalImports


##### Script
class BaseSLR1Parser():
    """
    The base class used for bottom-up `SLR(1)`_ parsing

    Parameters
    -----------
    productions: Union[List[Tuple[:class:`str`, List[:class:`str`]]], Dict[Hashable, Tuple[:class:`str`, List[:class:`str`]]]]
        The production rules of the `_CFG (Context Free Grammer)`_ :raw-html:`<br />` :raw-html:`<br />`

        The tuple for each production rule ``A --> B_1, ..., B_m`` contains:

        #. The non-terminal at the LHS of the production rule (A)
        #. The symbols that the rule produces (B_1, ..., B_m) :raw-html:`<br />` :raw-html:`<br />`

        If this argument is a dictionary, then the keys of the dictionary represent the id of a production rule.
        Otherwise, if this argument is a list, then assume that the ids of the production rules are the index position of the production rule :raw-html:`<br />` :raw-html:`<br />`

        .. important::
            Please read the important note at :attr:`productions`

    startSymbol: :class:`str`
        The starting non-terminal symbol

    startToken: :class:`str`
        The name of the starting token for an input string :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``STARTTOKEN``

    endToken: :class:`str`
        The name of the ending token for an input string :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``ENDTOKEN``

    nullToken: :class:`str`
        The name for the empty token :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``EPSILON``

    setup: :class:`bool`
        Whether to initialize all the setup for the parser automatically by calling :meth:`setup` :raw-html:`<br />` :raw-html:`<br />`

        **Default**: ``True``

    Attributes
    ----------
    startToken: :class:`str`
        The name of the starting token for an input string

    endToken: :class:`str`
        The name of the ending token for an input string

    nullToken: :class:`str`
        The name for the empty token

    nullable: Dict[:class:`str`, :class:`bool`]
        The `Nullable Set`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the non-terminal symbols and the values are whether each symbol is nullable

    first: Dict[:class:`str`, Set[:class:`str`]]
        The `First Set`_ for only each single non-terminal symbol :raw-html:`<br />` :raw-html:`<br />`

        The keys are the non-terminal symbols and the values are the possible terminal symbols that could
        appear in front of the particular non-terminal symbol

    follow: Dict[:class:`str`, Set[]]
        The `Follow Set`_ :raw-html:`<br />` :raw-html:`<br />`

        The keys are the non-terminal symbols and the values are the possible terminal symbols that could 
        appear after the particular non-terminal symbol
    """

    def __init__(self, productions: Union[List[Tuple[str, List[str]]], Dict[Hashable, Tuple[str, List[str]]]], startSymbol: str, 
                 startToken: str = "STARTTOKEN", endToken: str = "ENDTOKEN", nullToken: str = "EPSILON",
                 setup: bool = True):

        self.startToken = startToken
        self.endToken = endToken
        self.nullToken = nullToken

        self.productions = productions
        self._nonTermSymbols = self.getNonTermSymbols()
        self.startSymbol = startSymbol

        self.nullable: Dict[str, bool] = {}
        self.first: Dict[str, Set[str]] = {}
        self.follow: Dict[str, Set[str]] = {}

        self._dfa = DFA()
        self._reductions: Dict[Hashable, Union[int, Dict[str, int]]] = {}

        if (setup):
            self.setup()

    @property
    def productions(self) -> Dict[Hashable, Tuple[str, List[str]]]:
        """
        The production rules of the `_CFG (Context Free Grammer)`_ :raw-html:`<br />` :raw-html:`<br />`

        .. important::
            Assume that you have the following arguments:

            * ``startSymbol = "S_Prime"`` be the starting terminal and your actual starting non-terminal token for your `CFG`_ to be ``S``
            * ``startToken = "start"`` be some terminal token that denotes the starting of the input string
            * ``endToken = "end"`` be some terminal token that denotes the ending of the input string

            :raw-html:`<br />` :raw-html:`<br />`

            Please include the following production rule into this argument:

            ``("S_PRIME", ["start", "S", "end"])``

        :getter: Retrives the productions
        :setter: Sets the new production rules
        :type: Dict[Hashable, Tuple[:class:`str`, List[:class:`str`]]]
        """

        return self._productions
    
    @productions.setter
    def productions(self, newProductions: Union[List[Tuple[str, List[str]]], Dict[Hashable, Tuple[str, List[str]]]]):
        if (isinstance(newProductions, list)):
            newProductions = ListTools.toDict(newProductions)

        terminalSymbols = {self.startToken, self.endToken, self.nullToken}
        for prodId in newProductions:
            prodKey, prodVals = newProductions[prodId]
            if (prodKey in terminalSymbols):
                raise KeyError(f"{prodKey} cannot appear on the LHS of the production, {prodKey} --> {prodVals} , since {prodKey} is a terminal symbol")
            
        self._productions = newProductions
        self._nonTermSymbols = self.getNonTermSymbols()

    @property
    def nonTermSymbols(self) -> Set[str]:
        """
        The set of non-terminal symbols of the `CFG`_

        :getter: Retrives the non-terminal symbols
        :type: Set[:class:`str`]
        """

        return self._nonTermSymbols
    
    @property
    def startSymbol(self) -> str:
        """
        The starting non-terminal symbol

        :getter: Retrives the starting non-terminal symbol
        :setter: Sets the new starting non-terminal symbol
        :type: :class:`str`
        """

        return self._startSymbol
    
    @startSymbol.setter
    def startSymbol(self, newStartSymbol: str) -> str:
        if (newStartSymbol not in self._nonTermSymbols):
            raise KeyError(f"The start symbol, {newStartSymbol} is not a valid non-terminal symbol since it does not belong in the following set of non-terminal symbols {self._nonTermSymbols}")
        
        self._startSymbol = newStartSymbol

    def clear(self):
        """
        Clears all the setup from the parser
        """

        self.nullable.clear()
        self.first.clear()
        self.follow.clear()

        self._dfa.clear()
    
    def getNonTermSymbols(self) -> Set[str]:
        """
        Retrieves the set of non-terminal symbols of the `CFG`_

        Returns
        -------
        Set[:class:`str`]
            The set of non-terminal symbols
        """

        result = set()
        for prodId in self._productions:
            prodKey, _ = self._productions[prodId]
            result.add(prodKey)

        return result

    def getNullableSet(self) -> Dict[str, bool]:
        """
        Computes the `Nullable Set`_

        Returns
        -------
        Dict[:class:`str`, :class:`bool`]
            Whether each non-terminal symbol is nullable :raw-html:`<br />` :raw-html:`<br />`

            The keys are the non-terminal symbols and the values are whether each symbol is nullable
        """
        
        result = defaultdict(lambda: False)
        for symbol in self.nonTermSymbols:
            result[symbol] = False

        hasChange = True
        while (hasChange):
            hasChange = False
            for prodId in self._productions:
                prodKey, prodVals = self._productions[prodId]
                if (result[prodKey]):
                    continue

                allNullable = True
                for val in prodVals:
                    if (val != self.nullToken and (val not in self.nonTermSymbols or not result[val])):
                        allNullable = False
                        break

                if (not allNullable):
                    continue

                prevNullable = result[prodKey]
                result[prodKey] = True

                if (not hasChange and not prevNullable):
                    hasChange = True

        return dict(result)
    
    def getFirstSet(self, updateNullable: bool = True) -> Dict[str, Set[str]]:
        """
        Computes the `First Set`_ for only each single non-terminal symbol

        Parameters
        ----------
        updateNullable: :class:`bool`
            Whether to update the `Nullable Set`_ using :meth:`getNullableSet` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        Returns 
        -------
        Dict[:class:`str`, Set[:class:`str`]]
            The first terminal symbols to appear for a non-terminal symbol :raw-html:`<br />` :raw-html:`<br />`

            The keys are the non-terminal symbols and the values are the possible terminal symbols that could
            appear first for the particular non-terminal symbol
        """

        if (updateNullable):
            self.nullable = self.getNullableSet()

        result = defaultdict(lambda: set())
        hasChange = True

        while (hasChange):
            hasChange = False

            for prodId in self._productions:
                prodKey, prodVals = self._productions[prodId]
                prevFirstLen = len(result[prodKey])
                newFirst = self.getFirst(prodVals, self.nullable, result)
                result[prodKey] |= newFirst

                if (not hasChange and prevFirstLen != len(result[prodKey])):
                    hasChange = True

        return dict(result)
    
    def getFirst(self, symbols: List[str], nullable: Dict[str, bool], first: Dict[str, Set[str]]) -> Set[str]:
        """
        Retrieves the first terminal symbols to appear given a list of symbols

        Parameters
        ----------
        symbols: List[:class:`str`]
            The symbols to read

        nullable: Dict[:class:`str`, :class:`bool`]
            The `Nullable Set`_ :raw-html:`<br />` :raw-html:`<br />`

            The keys are the non-terminal symbols and the values are whether each symbol is nullable

        first: Dict[:class:`str`, Set[:class:`str`]]
            The `First Set`_ for only each single non-terminal symbol :raw-html:`<br />` :raw-html:`<br />`

            The keys are the non-terminal symbols and the values are the possible terminal symbols that could
            appear first for the particular non-terminal symbol
        """

        result = set()
        for symbol in symbols:
            if (symbol == self.nullToken):
                continue

            if (symbol not in self._nonTermSymbols):
                result.add(symbol)
                break

            result |= first[symbol]
            if (not nullable[symbol]):
                break

        return result

    def getFollowSet(self, updateNullable: bool = True, updateFirst: bool = True):
        """
        Computes the `Follow Set`_

        Parameters
        ----------
        updateNullable: :class:`bool`
            Whether to update the `Nullable Set`_ using :meth:`getNullableSet` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        updateFirst: :class:`bool`
            Whether to update the `First Set`_ using :meth:`getFirstSet` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``
        """

        if (updateNullable):
            self.nullable = self.getNullableSet()

        if (updateFirst):
            self.first = self.getFirstSet(updateNullable = False)

        firstVals = {}
        result = defaultdict(lambda: set())
        hasChange = True

        while (hasChange):
            hasChange = False

            for prodId in self._productions:
                prodKey, prodVals = self._productions[prodId]
                prodValsLen = len(prodVals)

                for i in range(prodValsLen):
                    val = prodVals[i]
                    if (val not in self.nonTermSymbols):
                        continue

                    prevFollowLen = len(result[val])
                    suffix = tuple(prodVals[i + 1:])

                    if (suffix not in firstVals):
                        firstVals[suffix] = self.getFirst(suffix, self.nullable, self.first)

                    result[val] |= firstVals[suffix]

                    allNullable = True
                    if (i < prodValsLen - 1):
                        for j in range(i + 1, prodValsLen):
                            currentVal = prodVals[j]
                            if (currentVal != self.nullToken and (currentVal not in self.nonTermSymbols or not self.nullable[currentVal])):
                                allNullable = False
                                break

                    if (allNullable):
                        result[val] |= result[prodKey]

                    if (not hasChange and len(result[val]) != prevFollowLen):
                        hasChange = True

        return dict(result)
    
    def _generateStateId(self) -> Hashable:
        return str(uuid.uuid4())
    
    def _generateProductionId(self) -> Hashable:
        return str(uuid.uuid4())

    def _addImpliedProductions(self, prodInds: List[Tuple[Hashable, Hashable]], bookmarks: DefaultDict[Hashable, int], 
                               statesByProds: DefaultDict[Hashable, Set[Hashable]], stateId: Optional[Hashable] = None):
        currentProdIndsLen = len(prodInds)
        uniqueProdInds = set()
        for prodId, prodInd in prodInds:
            bookmark = bookmarks[prodId]
            uniqueProdInds.add((prodInd, bookmark))

        i = 0
        while (i < currentProdIndsLen):
            prodId, prodInd = prodInds[i]
            prodKey, prodVals = self._productions[prodInd]
            bookmark = bookmarks[prodId]

            if (bookmark >= len(prodVals)):
                i += 1
                continue

            currentChar = prodVals[bookmark]
            if (currentChar not in self.nonTermSymbols):
                i += 1
                continue

            for newProdInd in self._productions:
                newProdKey, newProdVals = self._productions[newProdInd]
                if (newProdKey == currentChar and (newProdInd != prodInd or bookmark != 0) and (newProdInd, 0) not in uniqueProdInds):
                    newProdId = self._generateProductionId()
                    prodInds.append((newProdId, newProdInd))
                    uniqueProdInds.add((newProdInd, 0))

                    newBookmark = bookmarks[newProdId]
                    prodBookmarkId = (newProdInd, newBookmark)

                    if (stateId is not None):
                        statesByProds[prodBookmarkId].add(stateId)

                    currentProdIndsLen += 1

            i += 1

    def constructDFA(self, updateNullable: bool = True, updateFirst: bool = True, updateFollow: bool = True):
        """
        Constructs the `DFA`_ to determine whether to shift/reduce when reading the input

        Parameters
        ----------
        updateNullable: :class:`bool`
            Whether to update the `Nullable Set`_ using :meth:`getNullableSet` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        updateFirst: :class:`bool`
            Whether to update the `First Set`_ using :meth:`getFirstSet` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``

        updateFollow: :class:`bool`
            Whether to update the `Follow Set`_ using :meth:`getFollowSet` :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``True``
        """

        if (updateNullable):
            self.nullable = self.getNullableSet()

        if (updateFirst):
            self.first = self.getFirstSet(updateNullable = False)

        if (updateFollow):
            self.follow = self.getFollowSet(updateNullable = False, updateFirst = False)
        
        self._dfa.clear()
        currentStateId = self._generateStateId()
        statesByProds = defaultdict(lambda: set())
        bookmarks = defaultdict(lambda: 0)
        reductions = defaultdict(lambda: {})

        # get the starting productions
        states = {currentStateId: []}
        for prodInd in self._productions:
            prodKey, prodVals = self._productions[prodInd]
            if (prodKey == self.startSymbol):
                states[currentStateId].append((self._generateProductionId(), prodInd))

        stack = deque([(currentStateId, states[currentStateId])])
        self._addImpliedProductions(states[currentStateId], bookmarks, statesByProds, stateId = currentStateId)
        self._dfa.addState(currentStateId, isStart = True)

        while (stack):
            currentStateId, currentProdInds = stack.pop()
            currentProdIndsLen = len(currentProdInds)
            neighbours = defaultdict(lambda: [])

            for prodId, prodInd in currentProdInds:
                prodKey, prodVals = self._productions[prodInd]
                bookmark = bookmarks[prodId]
                prodValsLen = len(prodVals)

                # get neighbours
                toReduce = True
                while(bookmark < prodValsLen):
                    currentChar = prodVals[bookmark]

                    if (currentChar != self.nullToken):
                        neighbourProdId = self._generateProductionId()
                        neighbours[currentChar].append((neighbourProdId, prodInd))
                        bookmarks[neighbourProdId] = bookmark + 1
                        toReduce = False
                        break

                    bookmark += 1

                if (not toReduce):
                    continue
                
                # reduction
                if (currentProdIndsLen == 1):
                    reductions[currentStateId] = prodInd
                else:
                    followSet = self.follow[prodKey] if (prodKey in self.follow) else set()
                    for followSymbol in followSet:
                        reductions[currentStateId][followSymbol] = prodInd

                self._dfa.addState(currentStateId, isAccept = True)

            # add the neighbours
            for transitionSymbol in neighbours:
                existingNodeIds = None
                neighbourProdData = neighbours[transitionSymbol]
                self._addImpliedProductions(neighbourProdData, bookmarks, statesByProds)

                # check if neighbour does not already exist
                for prodId, prodInd in neighbourProdData:
                    bookmark = bookmarks[prodId]
                    prodBookmarkId = (prodInd, bookmark)
                    if (prodBookmarkId in statesByProds):
                        existingNodeIds = statesByProds[prodBookmarkId] if (existingNodeIds is None) else existingNodeIds & statesByProds[prodBookmarkId]

                neighbourExists = False
                if (existingNodeIds):
                    for neighbourId in existingNodeIds:
                        existingNodeProdData = states[neighbourId]
                        if (len(existingNodeProdData) != len(neighbourProdData)):
                            continue

                        existingNodeBookmarkIds = set()
                        for prodId, prodInd in existingNodeProdData:
                            existingNodeBookmarkIds.add((prodInd, bookmarks[prodId]))

                        neighbourBookmarkIds = set()
                        for prodId, prodInd in neighbourProdData:
                            neighbourBookmarkIds.add((prodInd, bookmarks[prodId]))

                        if (existingNodeBookmarkIds != neighbourBookmarkIds):
                            continue

                        self._dfa.addTransition(currentStateId, transitionSymbol, neighbourId)
                        neighbourExists = True
                        break
                
                if (neighbourExists):
                    continue
                
                # add the neighbour
                neighbourId = self._generateStateId()
                self._dfa.addTransition(currentStateId, transitionSymbol, neighbourId)
                stack.append((neighbourId, neighbourProdData))
                states[neighbourId] = neighbourProdData

                for prodId, prodInd in neighbourProdData:
                    bookmark = bookmarks[prodId]
                    prodBookmarkId = (prodInd, bookmark)
                    statesByProds[prodBookmarkId].add(neighbourId)

        self._reductions = dict(reductions)
        return states

    def setup(self):
        """
        Initializes any necessary setup for the parser
        """

        self.clear()
        self.nullable = self.getNullableSet()
        self.first = self.getFirstSet(updateNullable = False)
        self.follow = self.getFollowSet(updateFirst = False, updateNullable = False)
        self.constructDFA(updateNullable = False, updateFirst = False, updateFollow = False)

    @classmethod
    def _hasReduction(cls, currentIsAccept: bool, reductions: Union[int, Dict[str, int]], tokenType: str) -> bool:
        return currentIsAccept and reductions is not None and (not isinstance(reductions, dict) or tokenType in reductions)
    
    def _generateParserNodeId(self) -> Hashable:
        return str(uuid.uuid4())
    
    def _raiseSyntaxErr(self, ctx: ParseContext, token: Token):
        raise SyntaxErr(ctx, token, process = "parsing")

    def parse(self, tokens: Union[str, List[Token]], ctx: Optional[ParseContext] = None) -> ParseTree:
        """
        Parses an input text

        Parameters
        ----------
        tokens: Union[:class:`str`, List[:class:`Token`]]
            The tokenized tokens of the input text :raw-html:`<br />` :raw-html:`<br />`

            If this argument is a string, then will assume that each letter is an individual token :raw-html:`<br />` :raw-html:`<br />`

            Otheriwse, if this argument is a list, then each tuple contains
            
            #. the token type
            #. the value of the parsed token
            
            :raw-html:`<br />`

            .. note::
                Usually you can get the tokens of the input text by running some sort of tokenizer, such as
                :class:`BaseTokenizer`

        ctx: Optional[:class:`ParseContext`]
            The context for parsing :raw-html:`<br />` :raw-html:`<br />`

            **Default**: ``None``

        Raises
        ------
        :class:`SyntaxErr`
            If the parse tree cannot be constructed

        Returns
        -------
        :class:`ParserTree`
            The constructed parse tree
        """

        # create default context
        if (ctx is None):
            src = ""
            if (isinstance(tokens, list)):
                for token in tokens:
                    src += token.val
            else:
                src = tokens

            ctx = ParseContext(src)

        # convert the input string to tokens
        if (isinstance(tokens, str)):
            lines = tokens.splitlines(keepends = True)
            startLineNo = ctx.startLineNo
            newTokens = []

            linesLen = len(lines)
            for i in range(linesLen):
                line = lines[i]
                lineLen = len(line)

                for j in range(lineLen):
                    letter = line[j]
                    token = Token(letter, letter, startLineNo + i, j + 1)
                    newTokens.append(token)

            tokens = newTokens

        if (not tokens or tokens[0] != self.startToken):
            tokens.insert(0, Token(self.startToken, self.startToken, ctx.startLineNo, 0))

        if (not tokens or tokens[-1] != self.endToken):
            tokens.append(Token(self.endToken, self.endToken, ctx.getEndLineNo(), 0))
        
        self._dfa.reset()
        currentState = self._dfa.currentStateId
        currentIsAccept = self._dfa.isAccept(currentState)
        stateStack = deque([currentState])
        symbolStack = deque()

        treeNodes = {}
        treeChildren = {}
        treeNodeStack = deque()
        currentToken = None

        for token in tokens:
            currentToken = token
            reductions = self._reductions.get(currentState)
            hasReduction = self._hasReduction(currentIsAccept, reductions, token.type)

            while (hasReduction):
                prodInd = reductions if (not isinstance(reductions, dict)) else reductions[token.type]
                prodKey, prodVals = self._productions[prodInd]

                currentChildren = []
                for prodVal in prodVals:
                    symbolStack.pop()
                    stateStack.pop()
                    currentChildren.append(treeNodeStack.pop())

                currentChildren.reverse()
                self._dfa.currentStateId = stateStack[-1]
                currentState, currentIsAccept, transitionTaken = self._dfa.transition(prodKey)

                if (not transitionTaken):
                    self._raiseSyntaxErr(ctx, token)

                symbolStack.append(Token(prodKey, None, 0, 0))
                stateStack.append(currentState)

                parentId = self._generateParserNodeId()
                parentNode = ParseNode(parentId, prodId = prodInd)
                treeNodeStack.append(parentId)
                treeNodes[parentId] = parentNode
                treeChildren[parentId] = currentChildren

                reductions = self._reductions.get(currentState)
                hasReduction = self._hasReduction(currentIsAccept, reductions, token.type)

            currentState, currentIsAccept, transitionTaken = self._dfa.transition(token.type)
            if (not transitionTaken):
                self._raiseSyntaxErr(ctx, token)
            
            symbolStack.append(token)
            stateStack.append(currentState)

            nodeId = self._generateParserNodeId()
            node = ParseNode(nodeId, token = token)
            treeNodeStack.append(nodeId)
            treeNodes[nodeId] = node

        # last reduction to get the root node:
        tokenType = None if (currentToken is None) else currentToken.type
        reductions = self._reductions.get(currentState)
        hasReduction = self._hasReduction(currentIsAccept, reductions, tokenType)

        if (not hasReduction):
            self._raiseSyntaxErr(ctx, currentToken if (currentToken is not None) else Token(self.endToken, self.endToken, ctx.getEndLineNo(), 0))
        
        prodInd = reductions if (not isinstance(reductions, dict)) else reductions[tokenType]
        prodKey, prodVals = self._productions[prodInd]

        currentChildren = []
        for prodVal in prodVals:
            symbolStack.pop()
            currentState = stateStack.pop()
            currentChildren.append(treeNodeStack.pop())

        self._dfa.currentStateId = stateStack[-1]

        currentChildren.reverse()
        rootId = self._generateParserNodeId()
        root = ParseNode(rootId, prodId = prodInd)
        treeNodes[rootId] = root
        treeChildren[rootId] = currentChildren

        return ParseTree(treeNodes, treeChildren, rootId)
##### EndScript