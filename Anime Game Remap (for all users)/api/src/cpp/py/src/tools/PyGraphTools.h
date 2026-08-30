#ifndef AGRemapPyBind_PyGraphTools_H
#define AGRemapPyBind_PyGraphTools_H

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "PyTools.h"
#include "AGRemapCore/tools/GraphTools.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `Python`_-facing ``GraphTools`` -- a thin, fully-generic wrapper instantiating
 `AGRC::GraphTools`'s per-method templates with ``py::object``/`PyObjectHash`/`PyObjectEqual`
 :raw-html:`<br />` :raw-html:`<br />`

 Deliberately not tied to `AGRC::CallGraph`::Node or any other specific node shape -- see
 `AGRC::GraphTools`'s own note on this. A node can be any hashable `Python`_ value
 @endrst
 */
class PyGraphTools: public AGRC::GraphTools {
    public:
        using EdgeMap = std::unordered_map<py::object, std::vector<py::object>, PyObjectHash, PyObjectEqual>;
        using NodeSet = std::unordered_set<py::object, PyObjectHash, PyObjectEqual>;
        using FactMap = std::unordered_map<py::object, bool, PyObjectHash, PyObjectEqual>;
        using ForwardLocalFactMap = std::unordered_map<py::object, std::pair<bool, bool>, PyObjectHash, PyObjectEqual>;

        static NodeSet pyGetReachableNodes(const EdgeMap& forwardEdges, const NodeSet& rootNodes);

        static FactMap pyClampFactsToReachable(const FactMap& facts, const NodeSet& reachableNodes);

        static FactMap pyRunForwardMustFixpoint(const EdgeMap& forwardEdges, const EdgeMap& backwardEdges,
                                                  const NodeSet& rootNodes, const ForwardLocalFactMap& localFacts);

        static FactMap pyRunBackwardMustFixpoint(const EdgeMap& forwardEdges, const EdgeMap& backwardEdges,
                                                   const FactMap& localFacts);
};

void initCppGraphTools(pybind11::module_ &m);

#endif
