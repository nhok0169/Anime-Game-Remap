#include "PyIfTemplateNode.h"

#include <optional>
#include <utility>

#include <pybind11/stl.h>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// Shared with PyIfContentPart.cpp's own identical helper for 'id': None -> auto-generated,
// otherwise cast straight to size_t. Kept as a local copy (not shared via a header) since it's a
// one-line, self-contained helper -- the same convention parseId() itself already follows next to
// its own single call site rather than being pulled out into a common utility.
std::optional<size_t> parseNodeId(const py::object &id) {
    if (id.is_none()) {
        return std::nullopt;
    }
    return id.cast<size_t>();
}

// Wraps one PartsElement (ContentPart* | PyIfTemplateNode*) back into a py::object, reusing
// whichever existing pybind11 wrapper already represents that pointer (pybind11's own instance
// registry guarantees this, so object identity across repeated accesses is preserved -- the same
// property CallGraph/RegSurroundedAdd rely on for their own id(part) correlation, see the port's
// overall plan). 'reference' (not 'reference_internal') here since the *containing* list/dict this
// helper is used from is itself already bound with 'reference_internal' against the owning node --
// see this file's own top-level PyIfTemplateNode.h warning for the resulting, practical lifetime
// contract.
py::object partsElementToPy(const PyIfTemplateNode::PartsElement &element) {
    if (const PyIfContentPart* const* contentPart = std::get_if<PyIfContentPart*>(&element)) {
        return py::cast(*contentPart, py::return_value_policy::reference);
    }

    PyIfTemplateNode* const* childNode = std::get_if<PyIfTemplateNode*>(&element);
    return py::cast(*childNode, py::return_value_policy::reference);
}

}


void initCppIfTemplateNode(pybind11::module_ &m) {
    py::class_<PyIfTemplateNode>(m, "IfTemplateNode", R"doc(
A node within the parse tree of some :class:`IfTemplate`. This node contains a subset of the
:class:`IfContentPart`\s from the original :class:`IfTemplate`

.. note::
    For more details on the structure of the parse tree of an :class:`IfTemplate`, see
    :class:`IfTemplateTree`

.. warning::
    This node does not own the :class:`IfContentPart`/:class:`IfPredPart` instances referenced
    from :attr:`parts`/:attr:`ifPredPart` -- see this class's own binding header comment for the
    full lifetime contract. In short: keep whichever :class:`IfTemplate` this node's tree belongs
    to alive for as long as any reference obtained from this node is in use.

Parameters
----------
id: Optional[:class:`int`]
    The id for the node :raw-html:`<br />` :raw-html:`<br />`

    If this argument is ``None``, then will generate the id for the node

    **Default**: ``None``

ifPredPart: Optional[:class:`IfPredPart`]
    The predicate part that is associated with this node -- stored by reference, not copied; kept
    alive at least as long as this node (see this class's own top-level note)
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")

        .def(py::init([](const py::object &id, const py::object &ifPredPart) {
            AGRC::IfPredPart* actualIfPredPart = ifPredPart.is_none() ? nullptr : ifPredPart.cast<AGRC::IfPredPart*>();
            return std::make_unique<PyIfTemplateNode>(parseNodeId(id), actualIfPredPart);
        }), py::arg("id") = py::none(), py::arg("ifPredPart") = py::none(), py::keep_alive<1, 3>())

        .def_property_readonly("id", &PyIfTemplateNode::id,
    py::doc(R"doc(Hashable: The id for the node)doc"))

        .def_property_readonly("ifPredPart", [](PyIfTemplateNode &self) -> AGRC::IfPredPart* {
            return self.ifPredPart;
        }, py::return_value_policy::reference_internal,
    py::doc(R"doc(Optional[:class:`IfPredPart`]: The predicate part that is associated with this node)doc"))

        .def_property_readonly("children", [](PyIfTemplateNode &self) {
            py::dict result;
            for (const auto &entry : self.children()) {
                result[py::cast(entry.first)] = py::cast(entry.second, py::return_value_policy::reference);
            }
            return result;
        }, py::return_value_policy::reference_internal,
    py::doc(R"doc(Dict[:class:`int`, :class:`IfTemplateNode`]: The children to this node -- the keys are the ids of the children nodes and the values are the corresponding nodes for the children)doc"))

        .def_property_readonly("parts", [](PyIfTemplateNode &self) {
            py::list result;
            for (const PyIfTemplateNode::PartsElement &element : self.parts()) {
                result.append(partsElementToPy(element));
            }
            return result;
        }, py::return_value_policy::reference_internal,
    py::doc(R"doc(List[Union[:class:`IfContentPart`, :class:`IfTemplateNode`]]: The parts of the :class:`IfTemplate` within the node)doc"))

        .def("addChild", &PyIfTemplateNode::addChild, py::arg("node"), py::keep_alive<1, 2>(),
    py::doc(R"doc(
Adds a child to the node -- stored by reference, not copied; kept alive at least as long as this
node (see this class's own top-level note)

Parameters
----------
node: :class:`IfTemplateNode`
    The child to be added
        )doc"))

        .def("addIfContentPart", &PyIfTemplateNode::addIfContentPart, py::arg("part"), py::keep_alive<1, 2>(),
    py::doc(R"doc(
Adds an :class:`IfContentPart` to the node -- stored by reference, not copied; kept alive at least
as long as this node (see this class's own top-level note)

Parameters
----------
part: :class:`IfContentPart`
    The content part of the :class:`IfTemplate` to add to this node
        )doc"))

        .def("hasKey", &PyIfTemplateNode::hasKey, py::arg("key"),
    py::doc(R"doc(
Purely checks whether the key exists within the parts of the node without accounting for whether
the key exists in other subcommands called by this node or other children nodes that have the key

Parameters
----------
key: :class:`str`
    The key to check

Returns
-------
:class:`bool`
    Whether the key exists
        )doc"))

        .def("getKeyPart", &PyIfTemplateNode::getKeyPart, py::arg("key"), py::return_value_policy::reference_internal,
    py::doc(R"doc(
Retrieves the latest :class:`IfContentPart` that contains 'key'

Parameters
----------
key: :class:`str`
    The key to find

Returns
-------
Optional[:class:`IfContentPart`]
    The found part if available
        )doc"))

        .def("getKeyVal", [](PyIfTemplateNode &self, const std::string &key) -> py::object {
            std::optional<std::string> result = self.getKeyVal(key);
            return result.has_value() ? py::cast(*result) : py::object(py::none());
        }, py::arg("key"),
    py::doc(R"doc(
Retrieves the latest value that corresponds to 'key'

Parameters
----------
key: :class:`str`
    The key to find

Returns
-------
Optional[:class:`str`]
    The found value if available
        )doc"))

        .def("getKeyValues", &PyIfTemplateNode::getKeyValues, py::arg("key"),
    py::doc(R"doc(
Retrieves all the corresponding values to a certain key within the node

Parameters
----------
key: :class:`str`
    The key to find

Returns
-------
List[List[Tuple[:class:`int`, :class:`str`]]]
    All the corresponding values to the key in the node :raw-html:`<br />` :raw-html:`<br />`

    * The outer elements in the list are the values for each part in the node
    * The inner elements of the list are the different instance of the `KVP`_ within each part
    * The tuple contains the order index an occurence of the `KVP`_ appears in the part and the corresponding value for the `KVP`_
        )doc"))

        .def("getKeyMissingPart", [](PyIfTemplateNode &self, const std::string &key) {
            auto result = self.getKeyMissingPart(key);
            return py::make_tuple(
                result.first == nullptr ? py::object(py::none()) : py::cast(result.first, py::return_value_policy::reference),
                result.second
            );
        }, py::arg("key"),
    py::doc(R"doc(
Retrieves the first :class:`IfContentPart` if 'key' is not found in this node, without accounting
for the key being in any other subcommands or other children nodes

Parameters
----------
key: :class:`str`
    The key to find

Returns
-------
Tuple[Optional[:class:`IfContentPart`], :class:`bool`]
    A tuple containing:

    #. The first part found, if all the :class:`IfContentPart`\s within the node does not contain the key
    #. Whether a :class:`IfContentPart` is found within the node
        )doc"));
}
