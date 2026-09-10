#ifndef AGRemapPyBind_PyIfTemplateNode_H
#define AGRemapPyBind_PyIfTemplateNode_H

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

#include "PyIfContentPart.h"  // reuses PyIfContentPart/PyObjectHash/PyObjectEqual
#include "AGRemapCore/model/iftemplate/IfTemplateNode.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::IfTemplateNode`\\<py::object, py::object\\>. A plain
 alias, not a subclass -- ``id()`` is inherited from `AGRC::Node`\\<size_t\\> and bound directly
 on this type (safe: it only ever takes ``this`` as an implicit parameter -- see
 Architecture.md's note on this), matching how ``ParseNode`` binds its own inherited ``id()``.
 `AGRC::Node`\\<size_t\\> itself is not separately registered with `pybind11`_, for the same
 reason it isn't for ``ParseNode``. :raw-html:`<br />` :raw-html:`<br />`

 .. warning::
    This node does not own the :class:`IfContentPart`/:class:`IfPredPart`/:class:`IfTemplateNode`
    instances referenced from its ``parts``/``children``/``ifPredPart`` -- when built via
    :class:`IfTemplate`/:class:`IfTemplateTree` (the normal path), those are really owned by that
    :class:`IfTemplateTree`'s own internal node pool (plus, for content parts, the owning
    :class:`IfTemplate`'s own parts list), which is what actually keeps every reference in this
    tree alive. When a node is built directly from Python instead (bypassing
    :class:`IfTemplate`/:class:`IfTemplateTree`), ``addChild``/``addIfContentPart`` and this
    constructor's own ``ifPredPart`` parameter are all bound with a `pybind11`_
    ``keep_alive<1, N>`` policy, so the referenced object's own Python wrapper is kept alive for as
    long as the node referencing it is -- no caveat needed for that path either.
 @endrst
 */
using PyIfTemplateNode = AGRC::IfTemplateNode<std::string, std::string>;


void initCppIfTemplateNode(pybind11::module_ &m);

#endif
