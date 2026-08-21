#include "PyParseTree.h"

#include <stdexcept>
#include <string>

#include <pybind11/stl.h>

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    PyParseTree makeParseTree(PyParseTree::Nodes nodes, PyParseTree::Children children, py::object rootId) {
        try {
            return PyParseTree(std::move(nodes), std::move(children), std::move(rootId));
        } catch (const std::invalid_argument& e) {
            throw py::key_error(e.what());
        }
    }

    void setRootId(PyParseTree& self, py::object newRootId) {
        try {
            self.setRootId(std::move(newRootId));
        } catch (const std::invalid_argument& e) {
            throw py::key_error(e.what());
        }
    }

    // Restores the pure-Python original's 'default' parameter on top of the core method (which
    // deliberately dropped it in favor of a plain nullptr/found bool -- see ParseTree::getNode's
    // own doc comment for why). Cheap to layer back on here since Python callers expect it.
    py::object getNode(PyParseTree& self, const py::object& nodeId, bool errorOnNotFound, py::object defaultVal) {
        const PyParseNode* node = self.getNode(nodeId, false);
        if (node != nullptr) {
            return py::cast(*node);
        }
        if (errorOnNotFound) {
            throw py::key_error("Node Id by the id, '" + py::str(nodeId).cast<std::string>() + "', not found in the parse tree");
        }
        return defaultVal;
    }

}


void initCppParseTree(pybind11::module_ &m) {
    py::class_<PyParseTree>(m, "ParseTree", R"doc(
The generated parse tree after parsing some text

Parameters
----------
nodes: Dict[Hashable, :class:`ParseNode`]
    The nodes in the tree

    The keys are the ids of the node and the values are the nodes

children: Dict[Hashable, List[Hashable]]
    The children relations of the nodes

    The keys are the ids of the parent nodes and the values are the ids of the children nodes

rootId: Hashable
    The id of the root node
    )doc")

        .def(py::init(&makeParseTree), py::arg("nodes"), py::arg("children"), py::arg("rootId"))

        .def_readwrite("nodes", &PyParseTree::nodes, py::doc(R"doc(
Dict[Hashable, :class:`ParseNode`]: The nodes in the tree

The keys are the ids of the node and the values are the nodes
        )doc"))

        .def_readwrite("children", &PyParseTree::children, py::doc(R"doc(
Dict[Hashable, List[Hashable]]: The children relations of the nodes

The keys are the ids of the parent nodes and the values are the ids of the children nodes
        )doc"))

        .def_property("rootId", &PyParseTree::rootId, &setRootId, py::doc(R"doc(
Hashable: The id of the root node

:getter: Retrieves the id of the root node
:setter: Sets the new id of the root node
        )doc"))

        .def("getNode", &getNode, py::arg("nodeId"), py::arg("errorOnNotFound") = true, py::arg("default") = py::none(), py::doc(R"doc(
Retrieves a node based on the passed id

Parameters
----------
nodeId: Hashable
    The node id to search for

errorOnNotFound: :class:`bool`
    Whether to raise if no matching node is found

    **Default**: ``True``

default: Any
    The default value to return if no matching node is found and 'errorOnNotFound' is ``False``

    **Default**: ``None``

Raises
------
:class:`KeyError`
    No matching node is found and 'errorOnNotFound' is ``True``

Returns
-------
Optional[:class:`ParseNode`]
    The corresponding node, if found
        )doc"))

        .def("isChild", &PyParseTree::isChild, py::arg("nodeId"), py::doc(R"doc(
Determines whether the id of some node has no children of its own

Parameters
----------
nodeId: Hashable
    The id of the node

Returns
-------
:class:`bool`
    Whether the id corresponds to a node with no children of its own
        )doc"));
}
