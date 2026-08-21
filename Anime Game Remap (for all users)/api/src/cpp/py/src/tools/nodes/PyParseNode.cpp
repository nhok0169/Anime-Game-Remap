#include "PyParseNode.h"

#include <optional>

#include <pybind11/stl.h>

#include "AGRemapCore/tools/parsing/Token.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppParseNode(pybind11::module_ &m) {
    py::class_<PyParseNode>(m, "ParseNode", R"doc(
A node within a parse tree, created from a parser that interprets some `CFG`_

Parameters
----------
id: Hashable
    The id for the node

prodId: Optional[Hashable]
    The id for the chosen production from the `CFG`_

    **Default**: ``None``

token: Optional[:class:`Token`]
    The token that this node references

    **Default**: ``None``
    )doc")

        .def(py::init<py::object, std::optional<py::object>, std::optional<AGRC::Token>>(),
             py::arg("id"), py::arg("prodId") = py::none(), py::arg("token") = py::none())

        .def_property_readonly("id", &PyParseNode::id, py::doc(R"doc(
Hashable: The id of the node
        )doc"))

        .def_readwrite("prodId", &PyParseNode::prodId, py::doc(R"doc(
Optional[Hashable]: The id for the chosen production from the `CFG`_
        )doc"))

        .def_readwrite("token", &PyParseNode::token, py::doc(R"doc(
Optional[:class:`Token`]: The token that this node references
        )doc"));
}
