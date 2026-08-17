import os
from pathlib import Path


# DLL resolution for windows
if os.name == "nt":
    os.add_dll_directory(str(Path(__file__).resolve().parent))

##### LocalImports

# --- C++ --------
from .core import CppAhoCorasickDFA
from .core import CppAlgo
from .core import CppIntTools
from .core import CppListTools
from .core import Ranges
from .core import CppTrie
from .core import DFA
from .core import OrderedMultiMap
from .core import OrderedMultiMapSqrt
from .core import RemappedKeyData
from .core import KeyRemapData
from .core import ReplaceList
from .core import ReplaceIf
from .core import OrderedMultiMapIterator
from .core import OrderedMultiMapSqrtIterator
from .core import IOrderedMultiMap
from .core import appendAllToOrderedMultiMap
from .core import IfTemplatePart
from .core import IfContentPart
from .core import IfContentPartColourChange
from .core import IfContentPartColouring
from .core import Hash64
from .core import Hash128
from .core import CppHashTools

# --- Cython -----
from .CyDictTools import CyDictTools
from .CyListTools import CyListTools
from .CyHashTools import CyHashTools
from .CyAlgo import CyAlgo

# --- Python -----
from .constants.BufDataTypes import BufDataTypes
from .constants.BufElementTypes import BufElementTypes
from .constants.BufFormatNames import BufFormatNames
from .constants.BufTypeNames import BufDataTypeNames, BufElementNames
from .constants.ByteSize import ByteSize
from .constants.Colours import Colours
from .constants.DownloadMode import DownloadMode
from .constants.ColourConsts import ColourConsts
from .constants.Colours import ColourRanges
from .constants.FileExt import FileExt
from .constants.FileTypes import FileTypes
from .constants.FileEncodings import FileEncodings
from .constants.FilePrefixes import FilePrefixes
from .constants.FileSuffixes import FileSuffixes
from .constants.FilePathConsts import FilePathConsts
from .constants.ImgFormats import ImgFormats
from .constants.IniConsts import IniKeywords, IniBoilerPlate, IniGraphModObjKeywords
from .constants.IniGraphReplaceMode import IniGraphReplaceMode
from .constants.GameTypeNames import GameTypeNames
from .constants.GIBuilder import GIBuilder
from .constants.GlobalClassifiers import GlobalClassifiers
from .constants.GlobalCompilerParts import GlobalCompilerParts
from .constants.GlobalIniClassifiers import GlobalIniClassifiers
from .constants.GlobalIniRemoveBuilders import GlobalIniRemoveBuilders
from .constants.GlobalPackageManager import GlobalPackageManager
from .constants.IfPredPartType import IfPredPartType
from .constants.BaseModTypeBuilder import BaseModTypeBuilder
from .constants.ModTypeNames import ModTypeNames
from .constants.ModTypes import ModTypes, ModTypeBuilder
from .constants.RegFillMissingMode import RegFillMissingMode
from .constants.TexConsts import TexMetadataNames

from .controller.enums.ShortCommandOpts import ShortCommandOpts
from .controller.enums.CommandOpts import CommandOpts

from .data.HashData import HashData
from .data.IndexData import IndexData
from .data.IniFixBuilderData import IniFixBuilderData
from .data.IniParseBuilderData import IniParseBuilderData
from .data.ModData import ModData
from .data.ModDataAssets import ModDataAssets
from .data.VGRemapData import VGRemapDataBuilder, vgRemapDataBuilder

from .exceptions.BadBufData import BadBufData
from .exceptions.BufFileNotRecognized import BufFileNotRecognized
from .exceptions.ConflictingOptions import ConflictingOptions
from .exceptions.DuplicateFileException import DuplicateFileException
from .exceptions.Error import Error
from .exceptions.FileException import FileException
from .exceptions.InvalidDownloadMode import InvalidDownloadMode
from .exceptions.InvalidModType import InvalidModType
from .exceptions.MissingFileException import MissingFileException
from .exceptions.NoModType import NoModType
from .exceptions.RemapMissingBlendFile import RemapMissingBlendFile
from .exceptions.SyntaxErr import SyntaxErr

from .model.assets.BaseModAssets import BaseModAssets
from .model.assets.Hashes import Hashes
from .model.assets.Indices import Indices
from .model.assets.VertexCounts import VertexCounts
from .model.assets.IniFixBuilderArgs import IniFixBuilderArgs
from .model.assets.IniParseBuilderArgs import IniParseBuilderArgs
from .model.assets.ModAssets import ModAssets
from .model.assets.ModDictAssets import ModDictAssets
from .model.assets.ModMappedAssets import ModMappedAssets
from .model.assets.VGRemaps import VGRemaps

from .model.buffers.BufDataType import BufDataType
from .model.buffers.BufElementType import BufElementType
from .model.buffers.BufFloat import BufBaseFloat, BufFloat, BufFloat16
from .model.buffers.BufInt import BufBaseInt, BufSignedInt, BufUnSignedInt
from .model.buffers.BufType import BufType
from .model.buffers.BufUnorm import BufUnorm

from .model.files.BlendFile import BlendFile
from .model.files.BufFile import BufFile
from .model.files.File import File
from .model.files.IniFile import IniFile
from .model.files.TextureFile import TextureFile

from .model.iniparserdicts import KeepFirstDict

from .model.strategies.bufEditors.BaseBufEditor import BaseBufEditor
from .model.strategies.bufEditors.BufEditor import BufEditor

from .model.strategies.iniClassifiers.BaseIniClassifier import BaseIniClassifier
from .model.strategies.iniClassifiers.BaseIniClassifierBuilder import BaseIniClassifierBuilder
from .model.strategies.iniClassifiers.IniClassifier import IniClassifier
from .model.strategies.iniClassifiers.IniClassifierBuilder import IniClassifierBuilder
from .model.strategies.iniClassifiers.IniClassifyStats import IniClassifyStats

from .model.strategies.iniClassifiers.states.IniClsAction import IniClsAction
from .model.strategies.iniClassifiers.states.IniClsActionArgs import IniClsActionArgs
from .model.strategies.iniClassifiers.states.IniClsCond import IniClsCond
from .model.strategies.iniClassifiers.states.IniClsTransitionVals import IniClsTransitionVals

from .model.strategies.iniFixers.graphEdits.BaseIniGraphEdit import BaseIniGraphEdit
from .model.strategies.iniFixers.graphEdits.RegFillMissing import RegFillMissing
from .model.strategies.iniFixers.graphEdits.RegSurroundedAdd import RegSurroundedAdd

from .model.strategies.iniFixers.graphGroupEdits.BaseIniGraphGroupEdit import BaseIniGraphGroupEdit
from .model.strategies.iniFixers.graphGroupEdits.BlendEdit import RemapBlendReplace
from .model.strategies.iniFixers.graphGroupEdits.GraphGroupEdit import GraphGroupEdit
from .model.strategies.iniFixers.graphGroupEdits.GraphGroupRemap import GraphGroupRemap
from .model.strategies.iniFixers.graphGroupEdits.ResEdit import BaseResEdit, ResIdentity, ResReplace, ResCreate
from .model.strategies.iniFixers.graphGroupEdits.ResRegCollect import ResRegCollect
from .model.strategies.iniFixers.graphGroupEdits.ResGroupCollect import ResGroupCollect
from .model.strategies.iniFixers.graphGroupEdits.TexEdit import TexCreate

from .model.strategies.iniFixers.BaseIniPartEdit import BaseIniPartEdit

# TOREMOVE
from .model.strategies.iniFixers.BaseIniFixerOld import BaseIniFixerOld
from .model.strategies.iniFixers.GIMIFixerOld import GIMIFixerOld
from .model.strategies.iniFixers.GIMIObjMergeFixerOld import GIMIObjMergeFixer
from .model.strategies.iniFixers.GIMIObjRegEditFixerOld import GIMIObjRegEditFixer
from .model.strategies.iniFixers.GIMIObjReplaceFixerOld import GIMIObjReplaceFixer
from .model.strategies.iniFixers.GIMIObjSplitFixerOld import GIMIObjSplitFixer

from .model.strategies.iniFixers.GIMIFixer import GIMIFixer
from .model.strategies.iniFixers.IniFixBuilder import IniFixBuilder
from .model.strategies.iniFixers.MultiModFixer import MultiModFixer

# TOREMOVE
from .model.strategies.iniFixers.regEditFilters.BaseRegEditFilter import BaseRegEditFilter
from .model.strategies.iniFixers.regEditFilters.RegEditFilter import RegEditFilter
from .model.strategies.iniFixers.regEditFilters.RegNewVals import OldRegNewVals
from .model.strategies.iniFixers.regEditFilters.RegRemap import RegRemap
from .model.strategies.iniFixers.regEditFilters.RegRemove import RegRemove
from .model.strategies.iniFixers.regEditFilters.RegTexAdd import RegTexAdd
from .model.strategies.iniFixers.regEditFilters.RegTexEdit import RegTexEdit

from .model.strategies.iniFixers.regEdits.BaseRegEdit import BaseRegEdit
from .model.strategies.iniFixers.regEdits.RegNewVals import RegNewVals

# TOREMOVE
from .model.strategies.iniParsers.GIMIObjParserOld import GIMIObjParser

from .model.strategies.iniParsers.BaseIniParser import BaseIniParser
from .model.strategies.iniParsers.GIMIParser import GIMIParser, GIMISectionClassifier
from .model.strategies.iniParsers.IniParseBuilder import IniParseBuilder

from .model.strategies.iniRemovers.BaseIniRemover import BaseIniRemover
from .model.strategies.iniRemovers.IniRemover import IniRemover
from .model.strategies.iniRemovers.IniRemoveBuilder import IniRemoveBuilder

from .model.strategies.texEditors.pixelTransforms.BasePixelTransform import BasePixelTransform
from .model.strategies.texEditors.pixelTransforms.ColourReplace import ColourReplace
from .model.strategies.texEditors.pixelTransforms.CorrectGamma import CorrectGamma
from .model.strategies.texEditors.pixelTransforms.InvertAlpha import InvertAlpha
from .model.strategies.texEditors.pixelTransforms.HighlightShadow import HighlightShadow
from .model.strategies.texEditors.pixelTransforms.TempControl import TempControl
from .model.strategies.texEditors.pixelTransforms.TintTransform import TintTransform
from .model.strategies.texEditors.pixelTransforms.Transparency import Transparency

from .model.strategies.texEditors.texFilters.BaseTexFilter import BaseTexFilter
from .model.strategies.texEditors.texFilters.ColourReplaceFilter import ColourReplaceFilter
from .model.strategies.texEditors.texFilters.GammaFilter import GammaFilter
from .model.strategies.texEditors.texFilters.HueAdjust import HueAdjust
from .model.strategies.texEditors.texFilters.InvertAlphaFilter import InvertAlphaFilter
from .model.strategies.texEditors.texFilters.PixelFilter import PixelFilter
from .model.strategies.texEditors.texFilters.TexMetadataFilter import TexMetadataFilter
from .model.strategies.texEditors.texFilters.TransparencyAdjustFilter import TransparencyAdjustFilter

from .model.strategies.texEditors.BaseTexEditor import BaseTexEditor
from .model.strategies.texEditors.TexEditor import TexEditor
from .model.strategies.texEditors.TexCreator import TexCreator

from .model.strategies.ModType import ModType

from .model.iftemplate.IfPredLogicGenerator import IfPredLogicGenerator
from .model.iftemplate.SympyIfPredGenerator import SympyIfPredGenerator
from .model.iftemplate.IfPredParser import IfPredParser
from .model.iftemplate.SympyParser import SympyParser
from .model.iftemplate.IfPredPart import IfPredPart
from .model.iftemplate.IfPredTokenizer import IfPredTokenizer
from .model.iftemplate.SympyTokenizer import SympyTokenizer
from .model.iftemplate.IfTemplate import IfTemplate
from .model.iftemplate.IfTemplateNode import IfTemplateNode
from .model.iftemplate.IfTemplatePartOld import IfTemplatePartOld
from .model.iftemplate.IfTemplateTree import IfTemplateTree, IfTemplateNormTree, IfTemplateNonEmptyNodeTree

# TOREMOVE
from .model.iniresources.IniDownloadModel import IniDownloadModel
from .model.iniresources.IniFixResourceModel import IniFixResourceModel
from .model.iniresources.IniResourceModel import IniResourceModel
from .model.iniresources.IniSrcResourceModel import IniSrcResourceModel
from .model.iniresources.IniTexModel import IniTexModel

from .model.iniresources.IniGroupedResBuilder import IniGroupedResBuilder
from .model.iniresources.IniResource import IniResource, IniFixResource, IniGroupedResource
from .model.iniresources.RemapIniResource import RemapIniResource, RemapIniFixResource, RemapIniGroupedResource, RemapIniDownload
from .model.iniresources.RemapBlendResource import RemapBlendResource

from .model.textures.Colour import Colour
from .model.textures.ColourRange import ColourRange

from .model.stats.FileStats import FileStats
from .model.stats.CachedFileStats import CachedFileStats
from .model.stats.RemapStats import RemapStats

from .model.CallGraph import CallGraph
from .model.DownloadData import DownloadData, BlendDownloadData
from .model.IniGraphGroup import IniGraphGroup
from .model.IniNamingTools import IniNamingTools
from .model.IniSectionGraph import IniSectionGraph
from .model.Mod import Mod
from .model.Model import Model
from .model.SectionIterData import SectionIterQueryData, SectionIterData
from .model.Version import Version
from .model.VGRemap import VGRemap

from .tools.caches.Cache import Cache
from .tools.caches.LRUCache import LruCache

from .tools.concurrency.ConcurrentManager import ConcurrentManager
from .tools.concurrency.ProcessManager import ProcessManager
from .tools.concurrency.ThreadManager import ThreadManager

from .tools.enums.DeferredEnum import DeferredEnum
from .tools.enums.StrEnum import StrEnum

from .tools.files.FileDownload import FileDownload
from .tools.files.FileService import FileService
from .tools.files.FilePath import FilePath

from .tools.nodes.Node import Node
from .tools.nodes.ParseNode import ParseNode

from .tools.parsing.BaseSLR1Parser import BaseSLR1Parser
from .tools.parsing.BaseTokenizer import BaseTokenizer
from .tools.parsing.FilteredTokenizer import FilteredTokenizer
from .tools.parsing.ParseContext import ParseContext
from .tools.parsing.ParseTree import ParseTree
from .tools.parsing.Token import Token

from .tools.tries.AhoCorasicDFA import AhoCorasickDFA
from .tools.tries.AhoCorasickBuilder import AhoCorasickBuilder
from .tools.tries.AhoCorasickSingleton import AhoCorasickSingleton
from .tools.tries.BaseAhoCorasickDFA import BaseAhoCorasickDFA
from .tools.tries.PyWrapAhoCorasickDFA import PyWrapAhoCorasickDFA
from .tools.tries.Trie import Trie

from .tools.Algo import Algo
from .tools.Builder import Builder
from .tools.DictTools import DictTools
from .tools.FlyweightBuilder import FlyweightBuilder
from .tools.GraphTools import GraphTools
from .tools.Heading import Heading
from .tools.HeapNode import HeapNode
from .tools.IntTools import IntTools
from .tools.HashTools import HashTools
from .tools.ListTools import ListTools
from .tools.PackageManager import PackageManager
from .tools.PackageData import PackageData
from .tools.TextTools import TextTools

from .view.Logger import Logger

from .remapService import RemapService

from .main import remapMain
##### EndLocalImports

__all__ = ["CppListTools", "CppIntTools", "Ranges", "CppTrie", "CppAhoCorasickDFA", "CppAlgo",
           "OrderedMultiMap", "OrderedMultiMapSqrt", "RemappedKeyData", "KeyRemapData", "ReplaceList", "ReplaceIf", "OrderedMultiMapIterator", "OrderedMultiMapSqrtIterator", "IOrderedMultiMap", "appendAllToOrderedMultiMap", "IfTemplatePart", "IfContentPart", "IfContentPartColourChange", "IfContentPartColouring", "Hash64", "Hash128", "CppHashTools",
            
           "CyDictTools", "CyListTools", "CyHashTools", "CyAlgo",

           "BufDataTypes", "BufElementTypes", "BufFormatNames", "BufDataTypeNames", "BufElementNames", "ByteSize", "Colours", "DownloadMode", "ColourConsts", "ColourRanges",  "FileExt", "FileTypes", "FileEncodings", "FilePrefixes", "FileSuffixes", "FilePathConsts", "ImgFormats", "IniGraphModObjKeywords", "IniKeywords", "IniBoilerPlate", "IniGraphReplaceMode", "GameTypeNames", "GIBuilder", "GlobalClassifiers", "GlobalCompilerParts", "GlobalIniClassifiers", "GlobalIniRemoveBuilders", "GlobalPackageManager", "IfPredPartType", "BaseModTypeBuilder", "ModTypeNames", "ModTypes", "ModTypeBuilder", "TexMetadataNames", "RegFillMissingMode", 
           "ShortCommandOpts", "CommandOpts",
           "HashData", "IndexData", "IniFixBuilderData", "IniParseBuilderData", "ModData", "ModDataAssets", "VGRemapDataBuilder", "vgRemapDataBuilder",
           "BadBufData", "BufFileNotRecognized", "ConflictingOptions", "DuplicateFileException", "Error", "FileException", "InvalidDownloadMode",
           "InvalidModType", "MissingFileException", "NoModType", "RemapMissingBlendFile", "SyntaxErr",
           "BaseModAssets", "Hashes", "Indices", "VertexCounts", "IniFixBuilderArgs", "IniParseBuilderArgs", "ModAssets", "ModDictAssets", "ModMappedAssets", "VGRemaps",
           "BufDataType", "BufElementType", "BufBaseFloat", "BufFloat", "BufFloat16", "BufBaseInt", "BufSignedInt", "BufUnSignedInt", "BufType", "BufUnorm",
           "BlendFile", "BufFile", "File", "IniFile", "TextureFile",
           "KeepFirstDict",
           "BaseBufEditor", "BufEditor",
           "IniClsAction", "IniClsActionArgs", "IniClsCond", "IniClsTransitionVals",
           "BaseIniGraphEdit", "RegFillMissing", "RegSurroundedAdd",
           "BaseIniGraphGroupEdit", "GraphGroupEdit", "GraphGroupRemap", "BaseResEdit", "ResIdentity", "ResReplace", "ResCreate", "RemapBlendReplace", "ResRegCollect", "ResGroupCollect", "TexCreate",
           "BaseIniClassifier", "BaseIniClassifierBuilder", "IniClassifier", "IniClassifierBuilder", "IniClassifyStats", 
           "BaseIniPartEdit", "BaseIniFixerOld", "GIMIFixer", "GIMIFixerOld", "GIMIObjMergeFixer", "GIMIObjRegEditFixer", "GIMIObjReplaceFixer", "GIMIObjSplitFixer", "IniFixBuilder", "MultiModFixer",
           "BaseRegEditFilter", "RegEditFilter", "OldRegNewVals", "RegRemap", "RegRemove", "RegTexAdd", "RegTexEdit",
           "BaseRegEdit", "RegNewVals",
           "BaseIniParser", "GIMIObjParser", "GIMIParser", "GIMISectionClassifier", "IniParseBuilder",
           "BaseIniRemover", "IniRemover", "IniRemoveBuilder",
           "BasePixelTransform", "ColourReplace", "CorrectGamma", "InvertAlpha", "HighlightShadow", "TempControl", "TintTransform", "Transparency",
           "BaseTexFilter", "ColourReplaceFilter", "GammaFilter", "HueAdjust", "InvertAlphaFilter", "PixelFilter", "TexMetadataFilter", "TransparencyAdjustFilter",
           "BaseTexEditor", "TexEditor", "TexCreator",
           "ModType",
           "IfPredLogicGenerator", "SympyIfPredGenerator", "IfPredParser", "SympyParser", "IfPredPart", "IfPredTokenizer", "SympyTokenizer", "IfTemplate", "IfTemplateNode", "IfTemplatePartOld", "IfTemplateTree", "IfTemplateNormTree", "IfTemplateNonEmptyNodeTree",
           "IniDownloadModel", "IniFixResourceModel", "IniResourceModel", "IniSrcResourceModel", "IniTexModel",
           "IniGroupedResBuilder", "IniResource", "IniFixResource", "IniGroupedResource", "RemapIniResource", "RemapIniFixResource", "RemapIniGroupedResource", "RemapIniDownload", "RemapBlendResource",
           "Colour", "ColourRange",
           "FileStats", "CachedFileStats", "RemapStats",
           "CallGraph", "DownloadData", "BlendDownloadData", "IniGraphGroup", "IniNamingTools", "IniSectionGraph", "Mod", "Model", "SectionIterQueryData", "SectionIterData", "Version", "VGRemap",
           "Cache", "LruCache",
           "ConcurrentManager", "ProcessManager", "ThreadManager",
           "DeferredEnum", "StrEnum",
           "FileDownload", "FilePath", "FileService",
           "Node", "ParseNode",
           "BaseSLR1Parser", "BaseTokenizer", "FilteredTokenizer", "ParseContext", "ParseTree", "Token",
           "AhoCorasickDFA", "AhoCorasickBuilder", "AhoCorasickSingleton", "BaseAhoCorasickDFA", "PyWrapAhoCorasickDFA", "Trie",
           "Algo", "Builder", "DFA", "FlyweightBuilder", "DictTools", "GraphTools", "Heading", "HeapNode", "IntTools", "HashTools", "ListTools", "PackageManager", "PackageData", "TextTools",
           "Logger",
           "RemapService",
           "remapMain"]