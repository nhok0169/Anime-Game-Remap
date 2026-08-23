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
#include "model/strategies/PyModTypeIdData.h"
#include "model/strategies/iniClassifiers/PyIniClassifyStats.h"
#include "model/strategies/iniClassifiers/PyBaseIniClassifier.h"
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
    initCppIniClassifyStats(m);
    initCppBaseIniClassifier(m);
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
}