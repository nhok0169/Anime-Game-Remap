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
#include "tools/parsing/PyToken.h"
#include "tools/parsing/PyParseContext.h"
#include "tools/parsing/PyBaseTokenizer.h"
#include "tools/parsing/PyFilteredTokenizer.h"
#include "tools/parsing/PyIfPredTokenizer.h"
#include "tools/parsing/PySympyTokenizer.h"
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
    initCppToken(m);
    initCppParseContext(m);
    initCppBaseTokenizer(m);
    initCppFilteredTokenizer(m);
    initCppIfPredTokenizer(m);
    initCppSympyTokenizer(m);
    initCppHash64(m);
    initCppHash128(m);
    initCppHashTools(m);
}