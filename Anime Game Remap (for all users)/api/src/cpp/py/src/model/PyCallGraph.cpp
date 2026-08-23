#include "PyCallGraph.h"

#include <pybind11/stl.h>

#include "PyNodeIdentity.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

py::object nodeToPy(const PyCallGraph::Node &node) {
    py::object partId = py::cast(pyIdOfPart(node.part));
    if (node.isExit) {
        return py::make_tuple(py::cast(std::string("exit")), partId);
    }
    return partId;
}

py::dict edgeMapToPy(const PyCallGraph::EdgeMap &edges) {
    py::dict result;
    for (const auto &entry : edges) {
        py::list targets;
        for (const auto &target : entry.second) {
            targets.append(nodeToPy(target));
        }
        result[nodeToPy(entry.first)] = targets;
    }
    return result;
}

}


void initCppCallGraph(pybind11::module_ &m) {
    py::class_<PyCallGraph>(m, "CallGraph", R"doc(
The result of :meth:`IniSectionGraph.buildCallGraph` -- a `call graph`_ over the
:class:`IfContentPart`\s of an :class:`IniSectionGraph`, suitable for the `dataflow analysis`_
tools at :class:`GraphTools`

Nodes are either an integer equal to ``id(part)`` for the real :class:`IfContentPart`, or a virtual
``("exit", id(part))`` node (only present for a part that actually makes a ``run =`` call)
representing the point control reaches once that call has *returned* -- see :meth:`exitNodeOf`
    )doc")

        .def_property_readonly("forwardEdges", [](PyCallGraph &self) {
            return edgeMapToPy(self.forwardEdges());
        }, py::doc(R"doc(Dict[Any, List[Any]]: ``node -> list of the nodes that can run directly after it``)doc"))

        .def_property_readonly("backwardEdges", [](PyCallGraph &self) {
            return edgeMapToPy(self.backwardEdges());
        }, py::doc(R"doc(Dict[Any, List[Any]]: The reverse of :attr:`forwardEdges`)doc"))

        .def_property_readonly("partsById", [](PyCallGraph &self) {
            py::dict result;
            for (auto *part : self.parts()) {
                result[py::cast(pyIdOfPart(part))] = py::cast(part, py::return_value_policy::reference);
            }
            return result;
        }, py::doc(R"doc(Dict[:class:`int`, :class:`IfContentPart`]: The ``id()`` of every reachable :class:`IfContentPart`, mapped to the part itself)doc"))

        .def_property_readonly("rootNodeIds", [](PyCallGraph &self) {
            py::set result;
            for (auto *part : self.rootNodes()) {
                result.add(py::cast(pyIdOfPart(part)));
            }
            return result;
        }, py::doc(R"doc(Set[:class:`int`]: The ``id()`` of every part that's a genuine entry point of one of the graph's own target `section`_\s)doc"))

        .def("exitNodeOf", [](PyCallGraph &self, std::uintptr_t partId) -> py::object {
            for (auto *part : self.parts()) {
                if (pyIdOfPart(part) == partId) {
                    return nodeToPy(self.exitNodeOf(part));
                }
            }
            return py::cast(partId);
        }, py::arg("partId"),
    py::doc(R"doc(
Retrieves the node representing "once 'partId's own ``run =`` call (if it makes one) has returned"

For a part that makes no call, this is just 'partId' itself -- there's no call to distinguish
"before" from "after", so the part's own node already serves both purposes

Parameters
----------
partId: :class:`int`
    The ``id()`` of the :class:`IfContentPart` to look up (see :attr:`partsById`)

Returns
-------
Any
    Either ``("exit", partId)`` (if the part makes a ``run =`` call) or 'partId' itself
        )doc"));
}
