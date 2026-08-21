#ifndef AGRemapPyBind_PyParseNode_H
#define AGRemapPyBind_PyParseNode_H

#include <pybind11/pybind11.h>

#include "../PyTools.h"
#include "AGRemapCore/tools/nodes/ParseNode.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::ParseNode`\\<py::object\\>. A plain alias, not a
 subclass -- ``id()`` is inherited from :cpp:class:`AGRC::Node`\\<py::object\\> and bound
 directly on this type (safe: it only ever takes ``this`` as an implicit parameter, never the
 base type in an argument/return position -- see Architecture.md's note on this). :cpp:class:`AGRC::Node`
 itself is **not** separately registered with `pybind11`_ -- nothing needs a `ParseNode` to be
 `isinstance`-compatible with a bound ``Node`` type (the only other `Node` subclass in this
 codebase, ``IfTemplateNode``, is unrelated and still pure-Python)
 @endrst
 */
using PyParseNode = AGRC::ParseNode<py::object>;


void initCppParseNode(pybind11::module_ &m);

#endif
