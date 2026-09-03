#ifndef AGRemapPyBind_PyIfTemplateTree_H
#define AGRemapPyBind_PyIfTemplateTree_H

#include <pybind11/pybind11.h>

#include "PyIfContentPart.h"  // reuses PyObjectHash/PyObjectEqual
#include "AGRemapCore/model/iftemplate/IfTemplateTree.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::IfTemplateTree`\\<py::object, py::object\\> -- the parse
 tree for some :class:`IfTemplate`, always reached via :attr:`IfTemplate.tree`, never constructed
 directly from `Python`_. :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    `AGRC::IfTemplateNonEmptyNodeTree`/`AGRC::IfTemplateNormTree` (the two construction-algorithm
    variants `AGRC::IfTemplate` actually builds internally -- see that class's own binding) are
    deliberately **not** separately registered with `pybind11`_: neither adds any member beyond
    what `AGRC::IfTemplateTree` itself already has, so there is nothing `Python`_-visible to
    distinguish, and nothing in this codebase's real call sites ever needs
    ``isinstance(tree, IfTemplateNormTree)``-style discrimination between them (the pure-Python
    original's own ``treeCls`` was only ever used as a construction-time selector, never inspected
    afterward). A tree instance whose actual C++ dynamic type is one of the derived two still binds
    to Python as a plain ``IfTemplateTree`` -- safe, since it's always accessed by pointer/reference
    here, never sliced by value.
 @endrst
 */
using PyIfTemplateTree = AGRC::IfTemplateTree<std::string, std::string>;


void initCppIfTemplateTree(pybind11::module_ &m);

#endif
