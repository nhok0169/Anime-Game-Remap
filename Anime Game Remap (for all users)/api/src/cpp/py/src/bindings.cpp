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
}