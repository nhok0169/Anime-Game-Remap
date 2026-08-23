#include "PyIfTemplateTree.h"

#include "PyIfTemplateNode.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIfTemplateTree(pybind11::module_ &m) {
    py::class_<PyIfTemplateTree>(m, "IfTemplateTree", R"doc(
The parse tree for some :class:`IfTemplate`, reached via :attr:`IfTemplate.tree` -- never
constructed directly.

.. note::
    The nodes are the `IfContentPart`\s of the `IfTemplate`, wrapped in :class:`IfTemplateNode`\s
    forming the tree structure. See :class:`IfTemplateNode` for the parts/children each node holds.
    )doc")

        .def_property_readonly("root", [](PyIfTemplateTree &self) -> PyIfTemplateNode* {
            return self.root();
        }, py::return_value_policy::reference_internal,
    py::doc(R"doc(Optional[:class:`IfTemplateNode`]: The root node in the parse tree)doc"))

        .def("clear", &PyIfTemplateTree::clear,
    py::doc(R"doc(Clears the tree)doc"));
}
