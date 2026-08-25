#include <pybind11/pybind11.h>

#include <pybind11/pybind11.h>

#include "tools/PyListTools.h"
#include "tools/PyIntTools.h"
#include "tools/dfa/PyDFA.h"
#include "tools/PyBiMap.h"
#include "tools/PyAlgo.h"
#include "tools/PyRanges.h"
#include "tools/tries/PyTrie.h"
#include "tools/tries/PyAhoCorasickDFA.h"
#include "tools/orderedMultiMap/PyOrderedMultiMap.h"
#include "tools/orderedMultiMap/PyOrderedMultiMapSqrt.h"
#include "tools/orderedMultiMap/PyIOrderedMultiMap.h"
#include "model/iftemplate/PyIfContentPart.h"
#include "model/iftemplate/PyIfContentPartColour.h"
#include "model/PyVersion.h"
#include "model/assets/PyModDictAssets.h"
#include "model/assets/PyModMappedAssets.h"
#include "model/assets/PyModAssets.h"
#include "model/assets/PyHashes.h"
#include "model/assets/PyIndices.h"
#include "constants/PyGameTypeId.h"
#include "constants/PyModTypeId.h"
#include "constants/PyGIBuilder.h"
#include "model/strategies/PyModTypeIdData.h"
#include "model/strategies/PyModType.h"
#include "model/strategies/iniClassifiers/PyIniClassifyStats.h"
#include "model/strategies/iniClassifiers/PyBaseIniClassifier.h"
#include "model/strategies/iniClassifiers/PyIniClassifier.h"
#include "tools/parsing/PyToken.h"
#include "tools/parsing/PyParseContext.h"
#include "tools/parsing/PyBaseTokenizer.h"
#include "tools/parsing/PyFilteredTokenizer.h"
#include "tools/parsing/PyIfPredTokenizer.h"
#include "tools/parsing/PySympyTokenizer.h"
#include "tools/nodes/PyParseNode.h"
#include "tools/parsing/PyParseTree.h"
#include "tools/parsing/PyBaseSLR1Parser.h"
#include "model/iftemplate/PySympyParser.h"
#include "model/iftemplate/PyIfPredParser.h"
#include "tools/z3/PyZ3Context.h"
#include "tools/z3/PyZ3Predicate.h"
#include "model/iftemplate/PyIfPredPart.h"
#include "model/iftemplate/PyIfTemplateNode.h"
#include "model/iftemplate/PyIfTemplateTree.h"
#include "model/iftemplate/PyIfTemplate.h"
#include "model/PyCallGraph.h"
#include "model/PySectionIterData.h"
#include "model/PyIniSectionGraph.h"
#include "tools/hashing/PyHash64.h"
#include "tools/hashing/PyHash128.h"
#include "tools/hashing/PyHashTools.h"
#include "model/buffers/PyBufType.h"
#include "model/buffers/PyBufDataType.h"
#include "model/buffers/PyBufInt.h"
#include "model/buffers/PyBufFloat.h"
#include "model/buffers/PyBufUnorm.h"
#include "model/buffers/PyBufElementType.h"
#include "model/files/PyBinaryFile.h"
#include "model/files/PyBufFile.h"
#include "model/PyVGRemap.h"
#include "model/files/PyBlendFile.h"
#include "model/files/PyPositionFile.h"
#include "model/textures/PyColour.h"
#include "model/textures/PyColourRange.h"
#include "model/files/PyTextureFile.h"
#include "model/strategies/texEditors/pixelTransforms/PyBasePixelTransform.h"
#include "model/strategies/texEditors/pixelTransforms/PyCorrectGamma.h"
#include "model/strategies/texEditors/pixelTransforms/PyColourReplace.h"
#include "model/strategies/texEditors/pixelTransforms/PyHighlightShadow.h"
#include "model/strategies/texEditors/pixelTransforms/PyInvertAlpha.h"
#include "model/strategies/texEditors/pixelTransforms/PyTempControl.h"
#include "model/strategies/texEditors/pixelTransforms/PyTintTransform.h"
#include "model/strategies/texEditors/pixelTransforms/PyTransparency.h"
#include "model/strategies/texEditors/texFilters/PyBaseTexFilter.h"
#include "model/strategies/texEditors/texFilters/PyGammaFilter.h"
#include "model/strategies/texEditors/texFilters/PyColourReplaceFilter.h"
#include "model/strategies/texEditors/texFilters/PyTransparencyAdjustFilter.h"
#include "model/strategies/texEditors/texFilters/PyInvertAlphaFilter.h"
#include "model/strategies/texEditors/texFilters/PyHueAdjust.h"
#include "model/strategies/texEditors/texFilters/PyPixelFilter.h"
#include "model/strategies/texEditors/PyBaseTexEditor.h"
#include "model/strategies/texEditors/PyTexEditor.h"
#include "model/strategies/texEditors/PyTexCreator.h"

namespace py = pybind11;


PYBIND11_MODULE(core, m) {
    py::options options;
    options.enable_user_defined_docstrings();

    m.doc() = "C++ internal core of AGRemap";

    initCppListTools(m);
    initCppIntTools(m);
    initCppBiMap(m);
    initCppAlgo(m);
    initCppRanges(m);
    initCppDFA(m);
    initCppTrie(m);
    initCppAhoCorasickDFA(m);
    initCppOrderedMultiMap(m);
    initCppOrderedMultiMapSqrt(m);
    initCppIOrderedMultiMap(m);
    initCppIfContentPart(m);
    initCppIfContentPartColour(m);
    initCppVersion(m);
    initCppModDictAssets(m);
    initCppModMappedAssets(m);
    initCppModAssets(m);
    initCppHashes(m);
    initCppIndices(m);
    initCppGameTypeId(m);
    initCppModTypeId(m);
    initCppModTypeIdData(m);
    initCppModType(m);
    initCppGIBuilder(m); // must come after initCppModType (its methods return CppModType) and initCppModTypeId (uses the ModTypeId enum)
    initCppIniClassifyStats(m);
    initCppBaseIniClassifier(m);
    initCppIniClassifier(m); // must come after initCppBaseIniClassifier (registers its base)
    initCppToken(m);
    initCppParseContext(m);
    initCppBaseTokenizer(m);
    initCppFilteredTokenizer(m);
    initCppIfPredTokenizer(m);
    initCppSympyTokenizer(m);
    initCppParseNode(m);
    initCppParseTree(m);
    initCppBaseSLR1Parser(m);
    initCppSympyParser(m);
    initCppIfPredParser(m);
    initCppZ3Context(m);
    initCppZ3Predicate(m);
    initCppIfPredPart(m); // must come after initCppIfContentPart (registers its base, IfTemplatePart) and initCppZ3Context/initCppZ3Predicate
    initCppIfTemplateNode(m); // reuses PyIfContentPart/AGRC::IfPredPart in its own method signatures, so registered after both
    initCppIfTemplateTree(m); // its 'root' property returns IfTemplateNode, so registered after initCppIfTemplateNode
    initCppIfTemplate(m); // its 'tree' property returns IfTemplateTree, so registered after initCppIfTemplateTree
    initCppCallGraph(m);
    initCppSectionIterData(m);
    initCppIniSectionGraph(m);
    initCppHash64(m);
    initCppHash128(m);
    initCppHashTools(m);
    initCppBufType(m);
    initCppBufDataType(m); // must come after initCppBufType (registers its base)
    initCppBufInt(m); // must come after initCppBufDataType (registers its base)
    initCppBufFloat(m); // must come after initCppBufDataType (registers its base)
    initCppBufUnorm(m); // must come after initCppBufInt (registers its base, CppBufBaseInt)
    initCppBufElementType(m); // must come after initCppBufType and initCppBufDataType (constructor takes CppBufDataType instances)
    initCppBinaryFile(m);
    initCppBufFile(m); // must come after initCppBinaryFile/initCppBufElementType (registers its base / constructor arg type)
    initCppVGRemap(m);
    initCppBlendFile(m); // must come after initCppBufFile/initCppVGRemap
    initCppPositionFile(m); // must come after initCppBufFile
    initCppColour(m);
    initCppColourRange(m); // must come after initCppColour (constructor arg type)
    initCppTextureFile(m);
    initCppBasePixelTransform(m);
    initCppCorrectGamma(m); // must come after initCppBasePixelTransform (registers its base)
    initCppColourReplace(m); // must come after initCppBasePixelTransform/initCppColourRange
    initCppHighlightShadow(m); // must come after initCppBasePixelTransform (registers its base)
    initCppInvertAlpha(m); // must come after initCppBasePixelTransform (registers its base)
    initCppTempControl(m); // must come after initCppBasePixelTransform (registers its base)
    initCppTintTransform(m); // must come after initCppBasePixelTransform (registers its base)
    initCppTransparency(m); // must come after initCppBasePixelTransform (registers its base)
    initCppBaseTexFilter(m); // must come after initCppTextureFile (its 'transform' method signature references it)
    initCppGammaFilter(m); // must come after initCppBaseTexFilter (registers its base)
    initCppColourReplaceFilter(m); // must come after initCppBaseTexFilter/initCppColourRange
    initCppTransparencyAdjustFilter(m); // must come after initCppBaseTexFilter/initCppColourRange
    initCppInvertAlphaFilter(m); // must come after initCppBaseTexFilter (registers its base)
    initCppHueAdjust(m); // must come after initCppBaseTexFilter (registers its base)
    initCppPixelFilter(m); // must come after initCppBaseTexFilter/initCppBasePixelTransform
    initCppBaseTexEditor(m); // must come after initCppTextureFile (its 'fix' method signature references it)
    initCppTexEditor(m); // must come after initCppBaseTexEditor (registers its base)
    initCppTexCreator(m); // must come after initCppBaseTexEditor (registers its base) and initCppColour (constructor arg type)
}