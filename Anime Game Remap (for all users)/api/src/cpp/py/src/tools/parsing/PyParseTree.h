#ifndef AGRemapPyBind_PyParseTree_H
#define AGRemapPyBind_PyParseTree_H

#include <optional>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>

#include "../PyTools.h"
#include "../nodes/PyParseNode.h"
#include "AGRemapCore/tools/parsing/ParseTree.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief The `pybind11`_-facing name for `AGRC::ParseTree`\\<py::object, PyObjectHash, PyObjectEqual\\>
 */
using PyParseTree = AGRC::ParseTree<py::object, PyObjectHash, PyObjectEqual>;


/**
 * @brief
 @rst
 Converts a ``ParseTree<std::string, ...>`` (e.g. from :cpp:class:`AGRC::SympyParser`/
 :cpp:class:`AGRC::IfPredParser`, whose #Id is ``std::string``, not ``py::object``) into a
 :cpp:type:`PyParseTree`, so every `Python`_-visible parser -- regardless of its own internal
 #Id type -- returns the exact same `Python`_ ``ParseTree`` class from ``parse()``
 @endrst
 */
template <typename StringIdTree>
PyParseTree convertToPyParseTree(StringIdTree tree) {
    PyParseTree::Nodes nodes;
    nodes.reserve(tree.nodes.size());
    for (auto& [id, node] : tree.nodes) {
        std::optional<py::object> prodId = node.prodId.has_value() ? std::optional<py::object>(py::cast(*node.prodId)) : std::nullopt;
        nodes.emplace(py::cast(id), PyParseNode(py::cast(id), std::move(prodId), node.token));
    }

    PyParseTree::Children children;
    children.reserve(tree.children.size());
    for (auto& [id, kids] : tree.children) {
        std::vector<py::object> pyKids;
        pyKids.reserve(kids.size());
        for (const auto& kid : kids) {
            pyKids.push_back(py::cast(kid));
        }
        children.emplace(py::cast(id), std::move(pyKids));
    }

    return PyParseTree(std::move(nodes), std::move(children), py::cast(tree.rootId()));
}


void initCppParseTree(pybind11::module_ &m);

#endif
