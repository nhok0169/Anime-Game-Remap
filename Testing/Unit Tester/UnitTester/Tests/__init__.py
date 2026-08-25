from .test_AhoCorasickDFA import AhoCorasickDFATest
from .test_Algo import AlgoTest
from .test_BaseSLR1Parser import SLR1ParserTest
from .test_BaseTokenizer import BaseTokenizerTest
from .test_BinaryFile import BinaryFileTest
from .test_BlendFile import BlendFileTest
from .test_BufDataType import BufDataTypeTest
from .test_BufElementType import BufElementTypeTest
from .test_BufFile import BufFileTest
from .test_BufTools import BufToolsTest
from .test_CallGraph import CallGraphTest
from .test_ColourRange import ColourRangeTest
from .test_CppAhoCorasickDFA import CppAhoCorasickDFATest
from .test_CppBlendFile import CppBlendFileTest
from .test_CppBufFile import CppBufFileTest
from .test_CppGIBuilder import CppGIBuilderTest
from .test_CppHashTools import CppHashToolsTest
from .test_CppIfContentPart import CppIfContentPartTest
from .test_CppIniClassifier import CppIniClassifierTest
from .test_CppModType import CppModTypeTest
from .test_CppPositionFile import CppPositionFileTest
from .test_CppTrie import CppTrieTest
from .test_DFA import DFATest
from .test_DictTools import DictToolsTest
from .test_FileService import FileServiceTest
from .test_FilteredTokenizer import FilteredTokenizerTest
from .test_GraphTools import GraphToolsTest
from .test_Hash64 import Hash64Test
from .test_Hash128 import Hash128Test
from .test_HashTools import HashToolsTest
from .test_GIMIFixer import GIMIFixerTest
from .test_GIMIObjMergeFixer import GIMIObjMergeFixerTest
from .test_GIMIObjParser import GIMIObjParserTest
from .test_GIMIObjRegEditFixer import GIMIObjRegEditFixerTest
from .test_GIMIObjSplitFixer import GIMIObjSplitFixerTest
from .test_GIMIParser import GIMIParserTest
from .test_GraphGroupRemap import GraphGroupRemapTest
from .test_GraphInherit import GraphInheritTest
from .test_GraphRemove import GraphRemoveTest
from .test_GraphRename import GraphRenameTest
from .test_IfContentPart import IfContentPartTest
from .test_IfContentPartColour import IfContentPartColourTest, IfContentPartColourChangeTest
from .test_IOrderedMultiMap import IOrderedMultiMapTest
from .test_IfPredLogicGenerator import IfPredLogicGeneratorTest
from .test_IfPredParser import IfPredParserTest
from .test_IfPredPart import IfPredPartTest
from .test_IfPredTokenizer import IfPredTokenizerTest
from .test_IfTemplate import IfTemplateTest
from .test_IfTemplateNode import IfTemplateNodeTest
from .test_IfTemplateTree import IfTemplateTreeTest
from .test_IniClassifier import IniClassifierTest
from .test_IniFile import IniFileTest
from .test_IniFixResourceModel import IniFixResourceModelTest
from .test_IniRemover import IniRemoverTest
from .test_IniSectionGraph import IniSectionGraphTest
from .test_IntTools import IntToolsTest
from .test_ListTools import ListToolsTest
from .test_Logger import LoggerTest
from .test_Mod import ModTest
from .test_ModAssets import ModAssetsTest
from .test_ModDictAssets import ModDictAssetsTest
from .test_ModMappedAssets import ModMappedAssetsTest
from .test_Hashes import HashesTest
from .test_Indices import IndicesTest
from .test_ModType import ModTypeTest
from .test_ModTypeId import ModTypeIdTest
from .test_ModTypeIdData import ModTypeIdDataTest
from .test_ModTypes import ModTypesTest
from .test_MultiModFixer import MultiModFixerTest
from .test_OrderedMultiMap import OrderedMultiMapTest
from .test_OrderedMultiMapSqrt import OrderedMultiMapSqrtTest
from .test_OrderedMultiMapCrossCheck import OrderedMultiMapCrossCheckTest
from .test_ParseContext import ParseContextTest
from .test_PositionFile import PositionFileTest
from .test_PyWrapAhoCorasickDFA import PyWrapAhoCorasickDFATest
from .test_RegAdd import RegAddTest
from .test_RegRemap import RegRemapTest
from .test_RegRemove import RegRemoveTest
from .test_RegSurroundedAdd import RegSurroundedAddTest
from .test_RemapService import RemapServiceTest
from .test_ResGroupCollect import ResGroupCollectTest
from .test_ResRegCollect import ResRegCollectTest
from .test_SectionIterData import SectionIterDataTest
from .test_SympyIfPredGenerator import SympyIfPredGeneratorTest
from .test_SympyParser import SympyParserTest
from .test_SympyTokenizer import SympyTokenizerTest
from .test_Token import TokenTest
from .test_Trie import TrieTest
from .test_Version import VersionTest
from .test_VGRemap import VGRemapTest

from .test_TexEngine import TexEngineTest
from .test_CppColour import CppColourTest
from .test_CppTextureFile import CppTextureFileTest
from .test_TextureFile import TextureFileTest
from .test_CppBaseTexEditor import CppBaseTexEditorTest
from .test_BaseTexEditor import BaseTexEditorTest
from .test_CppBaseTexFilter import CppBaseTexFilterTest
from .test_CppGammaFilter import CppGammaFilterTest
from .test_CppBasePixelTransform import CppBasePixelTransformTest
from .test_CppCorrectGamma import CppCorrectGammaTest
from .test_CppColourReplace import CppColourReplaceTest
from .test_CppHighlightShadow import CppHighlightShadowTest
from .test_CppInvertAlpha import CppInvertAlphaTest
from .test_CppTempControl import CppTempControlTest
from .test_CppTintTransform import CppTintTransformTest
from .test_CppTransparency import CppTransparencyTest
from .test_CppColourReplaceFilter import CppColourReplaceFilterTest
from .test_CppTransparencyAdjustFilter import CppTransparencyAdjustFilterTest
from .test_CppInvertAlphaFilter import CppInvertAlphaFilterTest
from .test_CppHueAdjust import CppHueAdjustTest
from .test_CppPixelFilter import CppPixelFilterTest
from .test_PixelFilter import PixelFilterTest
from .test_CppTexEditor import CppTexEditorTest
from .test_TexEditor import TexEditorTest
from .test_CppTexCreator import CppTexCreatorTest
from .test_TexCreator import TexCreatorTest

__all__ = ["DictToolsTest", "FileServiceTest", "LoggerTest", "IniFixResourceModelTest", "IfTemplateTest", "IfTemplateNodeTest", "IniFileTest"]
__all__ += ["ModTest", "ModTypesTest", "ModTypeTest", "RemapServiceTest", "GIMIFixerTest", "GIMIParserTest", "IniRemoverTest"]
__all__ += ["GIMIObjParserTest", "GIMIObjMergeFixerTest", "GIMIObjSplitFixerTest", "MultiModFixerTest", "GIMIObjRegEditFixerTest"]
__all__ += ["TrieTest", "AlgoTest", "PyWrapAhoCorasickDFATest", "DFATest", "AhoCorasickDFATest", "IniClassifierTest", "IfTemplateTreeTest"]
__all__ += ["ColourRangeTest", "VersionTest", "IntToolsTest", "IfContentPartTest", "IfPredTokenizerTest", "SLR1ParserTest", "IfPredParserTest", "IfPredPartTest", "IfPredLogicGeneratorTest", "IniSectionGraphTest"]
__all__ += ["ModAssetsTest", "ModDictAssetsTest", "ModMappedAssetsTest", "HashesTest", "IndicesTest", "GraphGroupRemapTest", "GraphInheritTest", "GraphRemoveTest", "GraphRenameTest", "ResRegCollectTest", "ResGroupCollectTest", "SympyTokenizerTest", "SympyParserTest"]
__all__ += ["SympyIfPredGeneratorTest", "CppAhoCorasickDFATest", "CppTrieTest"]
__all__ += ["ModTypeIdDataTest", "CppModTypeTest", "CppGIBuilderTest", "ModTypeIdTest", "CppIniClassifierTest"]
__all__ += ["OrderedMultiMapTest", "OrderedMultiMapSqrtTest", "OrderedMultiMapCrossCheckTest", "IOrderedMultiMapTest", "CppIfContentPartTest"]
__all__ += ["IfContentPartColourTest", "IfContentPartColourChangeTest"]
__all__ += ["Hash64Test", "Hash128Test", "CppHashToolsTest", "HashToolsTest", "ListToolsTest"]
__all__ += ["RegAddTest", "RegRemapTest", "RegRemoveTest", "RegSurroundedAddTest"]
__all__ += ["TokenTest", "ParseContextTest", "BaseTokenizerTest", "FilteredTokenizerTest"]
__all__ += ["CallGraphTest", "SectionIterDataTest", "GraphToolsTest"]
__all__ += ["BufDataTypeTest", "BufElementTypeTest", "BinaryFileTest", "CppBufFileTest", "VGRemapTest", "CppBlendFileTest", "CppPositionFileTest", "BufToolsTest"]
__all__ += ["BufFileTest", "BlendFileTest", "PositionFileTest"]
__all__ += ["TexEngineTest", "CppColourTest", "CppTextureFileTest", "TextureFileTest"]
__all__ += ["CppBaseTexEditorTest", "BaseTexEditorTest", "CppBaseTexFilterTest", "CppGammaFilterTest"]
__all__ += ["CppBasePixelTransformTest", "CppCorrectGammaTest", "CppColourReplaceTest", "CppHighlightShadowTest", "CppInvertAlphaTest", "CppTempControlTest", "CppTintTransformTest", "CppTransparencyTest"]
__all__ += ["CppColourReplaceFilterTest", "CppTransparencyAdjustFilterTest", "CppInvertAlphaFilterTest", "CppHueAdjustTest", "CppPixelFilterTest", "PixelFilterTest"]
__all__ += ["CppTexEditorTest", "TexEditorTest", "CppTexCreatorTest", "TexCreatorTest"]