#include "PyGraphTools.h"

namespace py = pybind11;

PyGraphTools::NodeSet PyGraphTools::pyGetReachableNodes(const EdgeMap& forwardEdges, const NodeSet& rootNodes) {
    return AGRC::GraphTools::getReachableNodes<py::object, PyObjectHash, PyObjectEqual>(forwardEdges, rootNodes);
}

PyGraphTools::FactMap PyGraphTools::pyClampFactsToReachable(const FactMap& facts, const NodeSet& reachableNodes) {
    return AGRC::GraphTools::clampFactsToReachable<py::object, PyObjectHash, PyObjectEqual>(facts, reachableNodes);
}

PyGraphTools::FactMap PyGraphTools::pyRunForwardMustFixpoint(const EdgeMap& forwardEdges, const EdgeMap& backwardEdges,
                                                               const NodeSet& rootNodes, const ForwardLocalFactMap& localFacts) {
    return AGRC::GraphTools::runForwardMustFixpoint<py::object, PyObjectHash, PyObjectEqual>(forwardEdges, backwardEdges, rootNodes, localFacts);
}

PyGraphTools::FactMap PyGraphTools::pyRunBackwardMustFixpoint(const EdgeMap& forwardEdges, const EdgeMap& backwardEdges,
                                                                const FactMap& localFacts) {
    return AGRC::GraphTools::runBackwardMustFixpoint<py::object, PyObjectHash, PyObjectEqual>(forwardEdges, backwardEdges, localFacts);
}

void initCppGraphTools(pybind11::module_ &m) {
    py::class_<PyGraphTools>(m, "GraphTools", R"doc(
Tools for handling with generic directed graphs, represented as adjacency lists (``node -> list of
the nodes directly reachable from it``)

Nodes can be any hashable value -- these tools have no notion of what a node "is" (a section, an
IfContentPart, a plain str, ...); that meaning is entirely up to the caller.
    )doc")
        .def_static("getReachableNodes", &PyGraphTools::pyGetReachableNodes,
                    py::arg("forwardEdges"), py::arg("rootNodes"),
                    py::doc(R"doc(
Computes every node reachable from 'rootNodes', via plain forward graph reachability (BFS/DFS --
no dataflow facts involved, just "is there some path here at all").

Parameters
----------
forwardEdges: Dict[Any, List[Any]]
    The graph to search, as ``node -> list of the nodes directly reachable from it``.

rootNodes: Set[Any]
    The nodes to start searching from.

Returns
-------
Set[Any]
    Every node reachable from 'rootNodes' (including 'rootNodes' themselves).
                    )doc"))

        .def_static("clampFactsToReachable", &PyGraphTools::pyClampFactsToReachable,
                    py::arg("facts"), py::arg("reachableNodes"),
                    py::doc(R"doc(
Forces every fact about a node not in 'reachableNodes' down to False.

Parameters
----------
facts: Dict[Any, bool]
    The raw facts to clamp, as returned by runForwardMustFixpoint/runBackwardMustFixpoint.

reachableNodes: Set[Any]
    See getReachableNodes.

Returns
-------
Dict[Any, bool]
    The clamped facts.
                    )doc"))

        .def_static("runForwardMustFixpoint", &PyGraphTools::pyRunForwardMustFixpoint,
                    py::arg("forwardEdges"), py::arg("backwardEdges"), py::arg("rootNodes"), py::arg("localFacts"),
                    py::doc(R"doc(
Runs a forward, MUST (available-expressions-style) dataflow analysis over a graph, computing
whether some boolean property has been established entering every node -- correctly handling
cycles via fixpoint iteration (Kildall's/worklist algorithm).

Parameters
----------
forwardEdges: Dict[Any, List[Any]]
    The graph to analyze, as ``node -> list of the nodes that can run directly after it``.

backwardEdges: Dict[Any, List[Any]]
    The reverse of 'forwardEdges'.

rootNodes: Set[Any]
    The nodes that are true entry points of the graph.

localFacts: Dict[Any, Tuple[bool, bool]]
    For every node with content of its own worth examining, a tuple of (touches, localSatisfied).
    A node missing from this dict is treated as a pure pass-through.

Returns
-------
Dict[Any, bool]
    Every node reachable in the graph structure, mapped to whether the property is satisfied
    entering that node.
                    )doc"))

        .def_static("runBackwardMustFixpoint", &PyGraphTools::pyRunBackwardMustFixpoint,
                    py::arg("forwardEdges"), py::arg("backwardEdges"), py::arg("localFacts"),
                    py::doc(R"doc(
The mirror of runForwardMustFixpoint: a backward MUST (very-busy-expressions-style) dataflow
analysis over the same kind of graph, computing whether some boolean property is guaranteed to be
established somewhere after every node exits.

Parameters
----------
forwardEdges: Dict[Any, List[Any]]
    The graph to analyze, as ``node -> list of the nodes that can run directly after it``.

backwardEdges: Dict[Any, List[Any]]
    The reverse of 'forwardEdges'.

localFacts: Dict[Any, bool]
    For every node, whether its own content, by itself, already establishes the property being
    tracked. A node missing from this dict is treated as False.

Returns
-------
Dict[Any, bool]
    Every node reachable in the graph structure, mapped to whether the property is guaranteed to
    be satisfied somewhere after that node exits.
                    )doc"));
}
