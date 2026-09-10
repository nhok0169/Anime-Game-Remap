#ifndef AGRemapPyBind_PyGraphTools_H
#define AGRemapPyBind_PyGraphTools_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

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
