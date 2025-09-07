from .test_AhoCorasickDFA import AhoCorasickDFATest
from .test_Algo import AlgoTest
from .test_DFA import DFATest
from .test_DictTools import DictToolsTest
from .test_FastAhoCorasickDFA import FastAhoCorasickDFATest
from .test_FileService import FileServiceTest
from .test_GIMIFixer import GIMIFixerTest
from .test_GIMIObjMergeFixer import GIMIObjMergeFixerTest
from .test_GIMIObjRegEditFixer import GIMIObjRegEditFixerTest
from .test_GIMIObjSplitFixer import GIMIObjSplitFixerTest
from .test_GIMIParser import GIMIParserTest
from .test_GIMIObjParser import GIMIObjParserTest
from .test_IniClassifier import IniClassifierTest
from .test_IniRemover import IniRemoverTest
from .test_Logger import LoggerTest
from .test_IniFixResourceModel import IniFixResourceModelTest
from .test_IfTemplate import IfTemplateTest
from .test_IniFile import IniFileTest
from .test_Mod import ModTest
from .test_ModTypes import ModTypesTest
from .test_ModType import ModTypeTest
from .test_MultiModFixer import MultiModFixerTest
from .test_RemapService import RemapServiceTest
from .test_Trie import TrieTest
from .test_IfTemplateTree import IfTemplateTreeTest
from .test_IfTemplateNormTree import IfTemplateNormTreeTest
from .test_ColourRange import ColourRangeTest
from .test_Version import VersionTest
from .test_IntTools import IntToolsTest
from .test_IfContentPart import IfContentPartTest
from .test_IfPredTokenizer import IfPredTokenizerTest
#from .test_ModDictAssets import ModDictAssetsTest

__all__ = ["DictToolsTest", "FileServiceTest", "LoggerTest", "IniFixResourceModelTest", "IfTemplateTest", "IniFileTest"]
__all__ += ["ModTest", "ModTypesTest", "ModTypeTest", "RemapServiceTest", "GIMIFixerTest", "GIMIParserTest", "IniRemoverTest"]
__all__ += ["GIMIObjParserTest", "GIMIObjMergeFixerTest", "GIMIObjSplitFixerTest", "MultiModFixerTest", "GIMIObjRegEditFixerTest"]
__all__ += ["TrieTest", "AlgoTest", "FastAhoCorasickDFATest", "DFATest", "AhoCorasickDFATest", "IniClassifierTest", "IfTemplateTreeTest", "IfTemplateNormTreeTest"]
__all__ += ["ColourRangeTest", "VersionTest", "IntToolsTest", "IfContentPartTest", "IfPredTokenizerTest"]