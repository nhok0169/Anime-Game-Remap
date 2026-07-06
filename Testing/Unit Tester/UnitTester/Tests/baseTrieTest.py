import sys
from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class BaseTrieTest(BaseUnitTest):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls._trieData = {"shappy": "shappy value",
                        "shappyer": "shappyer value",
                        "s": "s value",
                        "app": "app value",
                        "apple": "apple value",
                        "appls": "appls value",
                        "applsy": "applsy value",
                        "pp": "pp value",
                        "le": "le value",
                        "pls": "pls value",
                        "plst": "plst value"}

        cls._makeTrie()
        
        cls._nodeId = 0
        cls._keywordId = 0

    @classmethod
    def _makeTrie(cls):
        pass
        
    def _getNextNodeId(self, currentNodeId: int) -> int:
        self._nodeId += 1
        return self._nodeId
    
    def _getNextKeywordId(self, currentKeywordId: int) -> int:
        self._keywordId += 1
        return self._keywordId
    
    def _resetNodeId(self) -> int:
        self._nodeId = -1
        self._trie._currentNodeId = self._nodeId
        return self._nodeId
    
    def _resetKeywordId(self) -> int:
        self._keywordId = -1
        self._trie._currentKeywordId = self._keywordId
        return self._keywordId
    
    def _setupMockFuncs(self):
        pass
        
    def setUp(self):
        super().setUp()
        self._setupMockFuncs()
        
        # TODO CTest
        # self.patch("src.py.FixRaidenBoss2.Trie._getNextNodeId", side_effect = lambda currentNodeId: self._getNextNodeId(currentNodeId))
        # self.patch("src.py.FixRaidenBoss2.Trie._getNextKeywordId", side_effect = lambda currentKeywordId: self._getNextKeywordId(currentKeywordId))
        # self.patch("src.py.FixRaidenBoss2.Trie._resetNodeId", side_effect = lambda: self._resetNodeId())
        # self.patch("src.py.FixRaidenBoss2.Trie._resetKeywordId", side_effect = lambda: self._resetKeywordId())

        self._makeTrie()


class BasePyTrieTest(BaseTrieTest):
    @classmethod
    def _makeTrie(cls):
        cls._trie = FRB.Trie(cls._trieData)

    def _setupMockFuncs(self):
        self.patch("src.py.FixRaidenBoss2.Trie._getNextNodeId", side_effect = lambda currentNodeId: self._getNextNodeId(currentNodeId))
        self.patch("src.py.FixRaidenBoss2.Trie._getNextKeywordId", side_effect = lambda currentKeywordId: self._getNextKeywordId(currentKeywordId))
        self.patch("src.py.FixRaidenBoss2.Trie._resetNodeId", side_effect = lambda: self._resetNodeId())
        self.patch("src.py.FixRaidenBoss2.Trie._resetKeywordId", side_effect = lambda: self._resetKeywordId())
