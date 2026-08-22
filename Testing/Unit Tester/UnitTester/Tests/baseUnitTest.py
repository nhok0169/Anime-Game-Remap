import sys
import unittest
from sympy import Equivalent
from sympy.logic.boolalg import Boolean
from unittest import mock
from typing import Dict, Any, Hashable, List, TypeVar, Set, Optional, Callable, Union
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB

T = TypeVar("T")

class PatchService:
    def _cleanup(self, patch, target):
        patch.stop()
        self.patches.pop(target)

    def patch(self, target, *args, **kwargs):
        p = mock.patch(target, *args, **kwargs)
        patchedMock = p.start()
        self.addCleanup(self._cleanup, *[p, target])
        self.patches[target] = patchedMock

    def patchObj(self, target, *args, **kwargs):
        p = mock.patch.object(target, *args, **kwargs)
        patchedMock = p.start()
        self.addCleanup(self._cleanup, *[p, target])
        self.patches[target] = patchedMock


class BaseUnitTest(unittest.TestCase, PatchService):
    @classmethod
    def setUpClass(cls):
        cls.patches: Dict[str, mock.Mock] = {}

    @classmethod
    def clearHashStates(cls):
        FRB.HashTools.clear()

    def setUp(self):
        self.clearHashStates()

    def getDataFailMsg(self, result: Any, expected: Any, msg: str):
        return f"{msg}\n\nresult: {result}\n\nexpected: {expected}"

    def compareDict(self, resultDict: Dict[Hashable, Any], expectedDict: Dict[Hashable, Any], compareValues: Optional[Callable[[Any, Any], None]] = None):
        resultDictLen = len(resultDict)
        expectedDictLen = len(expectedDict)
        if (resultDictLen != expectedDictLen):
            self.fail(self.getDataFailMsg(resultDict, expectedDict, f"Dictionaries have different lengths: resultDict: {resultDictLen}, expectedDict: {expectedDictLen}"))

        for resultKey in resultDict:
            resultValue = resultDict[resultKey]
            if (resultKey not in expectedDict):
                self.fail(self.getDataFailMsg(resultDict, expectedDict, f"The key, '{resultKey}', is not in the expected dictionary"))

            expectedValue = expectedDict[resultKey]

            if (compareValues is None and resultValue != expectedValue):
                self.fail(self.getDataFailMsg(resultDict, expectedDict, f"Different values for the key, {resultKey}, in both dictionaries: resultDict: {resultValue}, expectedDict: {expectedValue}"))
            elif (compareValues is not None):
                compareValues(resultValue, expectedValue)

    def compareList(self, resultList: List[T], expectedList: List[T], compareValues: Optional[Callable[[Any, Any], None]] = None):
        resultListLen = len(resultList)
        expectedListLen = len(expectedList)
        if (resultListLen != expectedListLen):
            self.fail(self.getDataFailMsg(resultList, expectedList, f"Lists have different lengths: resultList: {resultListLen}, expectedList: {expectedListLen}"))

        for i in range(resultListLen):
            resultValue = resultList[i]
            expectedValue = expectedList[i]
            if (compareValues is None and resultValue != expectedValue):
                self.fail(self.getDataFailMsg(resultList, expectedList, f"Different values in both lists: resultList: {resultValue}, expectedValue: {expectedValue}"))
            elif (compareValues is not None):
                compareValues(resultValue, expectedValue)

    def compareSet(self, resultSet: Set[T], expectedSet: Set[T]):
        resultSetLen = len(resultSet)
        expectedSetLen = len(expectedSet)
        if (resultSetLen != expectedSetLen):
            self.fail(self.getDataFailMsg(resultSet, expectedSet, f"Sets have different lengths: resultSet: {resultSetLen}, expectedSet: {expectedSetLen}"))

        resultSetDiff = resultSet - expectedSet
        if (resultSetDiff):
            self.fail(self.getDataFailMsg(resultSet, expectedSet, f"resultSet contains elements not in expectedSet: {resultSetDiff}"))

        expectedSetDiff = expectedSet - resultSet
        if (expectedSetDiff):
            self.fail(self.getDataFailMsg(resultSet, expectedSet, f"expectedSet contains elements not in resultSet: {expectedSetDiff}"))

    def compareDictList(self, resultDictLst: Dict[Hashable, List[Any]], expectedDictLst: Dict[Hashable, List[Any]], compareValues: Optional[Callable[[Any, Any], None]] = None):
        self.compareDict(resultDictLst, expectedDictLst, lambda resLst, expectedLst: self.compareList(resLst, expectedLst, compareValues = compareValues))

    def compareDictOfDict(self, resultDictOfDict: Dict[Hashable, Dict[Hashable, Any]], expectedDictOfDict: Dict[Hashable, Dict[Hashable, Any]], compareValues: Optional[Callable[[Any, Any], None]] = None):
        self.compareDict(resultDictOfDict, expectedDictOfDict, compareValues = lambda resultValue, expectedValue: self.compareDict(resultValue, expectedValue, compareValues = compareValues))

    def compareFileStats(self, resultFileStats: FRB.FileStats, expectedFileStats: FRB.FileStats):
        self.compareSet(resultFileStats.fixed, expectedFileStats.fixed)
        self.compareSet(resultFileStats.removed, expectedFileStats.removed)
        self.compareSet(resultFileStats.undoed, expectedFileStats.undoed)
        self.compareSet(resultFileStats.visitedAtRemoval, expectedFileStats.visitedAtRemoval)
        self.compareDict(resultFileStats.skipped, expectedFileStats.skipped, compareValues = lambda resultError, expectedError: self.assertEqual(type(resultError), type(expectedError)))
        self.compareDictOfDict(resultFileStats.skippedByMods, expectedFileStats.skippedByMods, compareValues = lambda resultError, expectedError: self.assertEqual(type(resultError), type(expectedError)))

    def compareParserNode(self, resultNode: FRB.ParseNode, expectedNode: FRB.ParseNode):
        self.assertEqual(resultNode.id, expectedNode.id)
        self.assertEqual(resultNode.prodId, expectedNode.prodId)

        self.assertEqual(type(resultNode.token), type(expectedNode.token))
        if (expectedNode.token is None):
            self.assertIsNone(resultNode.token)
        else:
            self.compareToken(resultNode.token, expectedNode.token)

    def compareParseTree(self, resultTree: FRB.ParseTree, expectedTree: FRB.ParseTree):
        self.assertEqual(resultTree.rootId, expectedTree.rootId)
        self.compareDictList(resultTree.children, expectedTree.children)

        self.compareSet(set(resultTree.nodes.keys()), expectedTree.nodes.keys())
        for nodeId in resultTree.nodes:
            self.compareParserNode(resultTree.nodes[nodeId], expectedTree.nodes[nodeId])

    def compareParserNodeShape(self, resultNode: FRB.ParseNode, expectedNode: FRB.ParseNode):
        """
        Same per-node comparison as compareParserNode, minus the id -- see compareParseTreeShape
        for why
        """

        self.assertEqual(resultNode.prodId, expectedNode.prodId)

        self.assertEqual(type(resultNode.token), type(expectedNode.token))
        if (expectedNode.token is None):
            self.assertIsNone(resultNode.token)
        else:
            self.compareToken(resultNode.token, expectedNode.token)

    def compareParseTreeShape(self, resultTree: FRB.ParseTree, expectedTree: FRB.ParseTree):
        """
        Compares two parse trees structurally (each node's prodId/token, and each node's children
        count/order) while deliberately ignoring the actual node ids -- unlike compareParseTree,
        which requires an exact id match.

        Needed for the C++-backed BaseSLR1Parser/SympyParser/IfPredParser: node ids are now
        generated as real random UUIDs (str(uuid.uuid4()) via the Python `uuid` module, called
        from the pybind11 binding's own default id generator), not deterministic sequential
        integers from a mocked generator the way the old pure-Python implementation's tests
        produced -- there is no way to patch/mock that generation from Python anymore (it isn't a
        patchable Python attribute on a compiled class), so expected trees can only be compared by
        shape, not by literal id equality. The expected-tree literals themselves are unchanged
        from the pure-Python era -- their own (also arbitrary) sequential integer ids only ever
        mattered as consistent labels tying a node to its position in `children`, which this
        walks structurally instead of by id lookup.
        """

        self._compareParseTreeShapeNode(resultTree, resultTree.rootId, expectedTree, expectedTree.rootId)

    def _compareParseTreeShapeNode(self, resultTree: FRB.ParseTree, resultNodeId: Hashable, expectedTree: FRB.ParseTree, expectedNodeId: Hashable):
        resultNode = resultTree.getNode(resultNodeId)
        expectedNode = expectedTree.getNode(expectedNodeId)
        self.compareParserNodeShape(resultNode, expectedNode)

        resultChildren = resultTree.children.get(resultNodeId, [])
        expectedChildren = expectedTree.children.get(expectedNodeId, [])
        self.assertEqual(len(resultChildren), len(expectedChildren), self.getDataFailMsg(resultChildren, expectedChildren, f"Different number of children for corresponding nodes '{resultNodeId}'/'{expectedNodeId}'"))

        for resultChildId, expectedChildId in zip(resultChildren, expectedChildren):
            self._compareParseTreeShapeNode(resultTree, resultChildId, expectedTree, expectedChildId)

    def compareParseCtx(self, resultCtx: FRB.ParseContext, expectedCtx: FRB.ParseContext):
        self.compareList(resultCtx.lines, expectedCtx.lines)
        self.assertEqual(resultCtx.file, expectedCtx.file)
        self.assertEqual(resultCtx.startLineNo, expectedCtx.startLineNo)

    def compareToken(self, resultToken: FRB.Token, expectedToken: FRB.Token):
        self.assertEqual(resultToken.type, expectedToken.type)
        self.assertEqual(resultToken.val, expectedToken.val)
        self.assertEqual(resultToken.lineNo, expectedToken.lineNo)
        self.assertEqual(resultToken.charNo, expectedToken.charNo)

    def compareSyntaxErr(self, resultSyntaxErr: FRB.SyntaxErr, expectedSyntaxErr: FRB.SyntaxErr):
        self.compareParseCtx(resultSyntaxErr.ctx, expectedSyntaxErr.ctx)
        self.compareToken(resultSyntaxErr.token, expectedSyntaxErr.token)

    def _compareIfTemplateTree(self, node: FRB.IfTemplateNode, rawTreeNode, ifTemplateParts: List[FRB.IfTemplatePart]):
        rawNodeIfPredInd = rawTreeNode[0]
        nodeIfPredPart = node.ifPredPart

        if (rawNodeIfPredInd is None):
            self.assertIsNone(nodeIfPredPart)
        else:
            self.assertIsInstance(nodeIfPredPart, FRB.IfPredPart)

        rawParts = rawTreeNode[1]
        nodeParts = node.parts

        rawPartsLen = len(rawParts)
        nodePartsLen = len(nodeParts)

        self.assertEqual(nodePartsLen, rawPartsLen)

        for i in range(nodePartsLen):
            rawPart = rawParts[i]
            nodePart = nodeParts[i]

            if (rawPart is None):
                self.assertIsInstance(nodePart, FRB.IfTemplateNode)
            else:
                self.assertIsInstance(nodePart, FRB.IfContentPart)

        rawChildren = rawTreeNode[2]
        nodeChildren = node.children

        rawChildrenLen = len(rawChildren)
        nodeChildrenLen = len(nodeChildren)
        self.assertEqual(nodeChildrenLen, rawChildrenLen)

        nodeChildrenKeys = list(nodeChildren.keys())

        for i in range(nodeChildrenLen):
            rawChild = rawChildren[i]

            nodeChildKey = nodeChildrenKeys[i]
            nodeChild = nodeChildren[nodeChildKey]
            self._compareIfTemplateTree(nodeChild, rawChild, ifTemplateParts)
        
    def compareIfTemplateTree(self, root: FRB.IfTemplateNode, rawTreeRoot, ifTemplateParts: List[FRB.IfTemplatePart]):
        self._compareIfTemplateTree(root,  rawTreeRoot, ifTemplateParts)

    def compareQuery(self, query1: Union[int, float, bool, Boolean], query2: Union[int, float, bool, Boolean]):
        self.assertEqual(type(query1), type(query2))

        if (isinstance(query1, int) or isinstance(query1, bool) or isinstance(query1, float)):
            self.assertEqual(query1, query2)

        self.assertEqual(Equivalent(query1, query2), True)

    def compareZ3Query(self, query1: FRB.Z3Predicate, query2: FRB.Z3Predicate):
        """
        Compares two :class:`Z3Predicate`\\s for logical equivalence, via a real `Z3`_ solver
        (``isSatisfiable()``) rather than string/structural comparison -- the same
        provable-equivalence pattern the C++ core's own ``core/tests/`` regression tests use
        (see ``IfPredPart_test.cpp``/``Z3IfPredGenerator_test.cpp``)

        .. note::
            The `Z3`_-flavored sibling of :meth:`compareQuery` (which stays `sympy`_-typed for the
            still-live `sympy`_-based subsystems, eg. :class:`IfPredLogicGenerator`) -- use this one
            for any query that came from the `Z3`_-based :class:`IfPredPart`/:class:`IniSectionGraph`
        """

        self.assertIsInstance(query1, FRB.Z3Predicate)
        self.assertIsInstance(query2, FRB.Z3Predicate)
        self.assertTrue(query1.sameContext(query2), f"Z3Predicates belong to different Z3Contexts, cannot compare: '{query1}' vs '{query2}'")

        notEquivalent = (query1 & ~query2).isSatisfiable() or (~query1 & query2).isSatisfiable()
        self.assertFalse(notEquivalent, f"Z3Predicates are not provably equivalent: '{query1}' vs '{query2}'")