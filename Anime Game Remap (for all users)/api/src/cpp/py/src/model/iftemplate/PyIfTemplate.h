#ifndef AGRemapPyBind_PyIfTemplate_H
#define AGRemapPyBind_PyIfTemplate_H

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
#include "AGRemapCore/model/iftemplate/IfTemplate.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::IfTemplate`\\<py::object, py::object\\>. A plain alias,
 not a subclass. Registered under the bare ``IfTemplate`` name (no ``Cpp`` prefix); the deprecated
 pure-Python original this replaced has been removed.

 :raw-html:`<br />`

 .. note::
    ``AGRC::IfTemplateRunConfig<std::string, std::string>::runKey``/``sectionNameOf`` (see
    ``IfTemplate.h``) are always ``py::str("run")`` and a plain ``py::object::cast<std::string>()``
    here -- these `.ini`_-domain customization points exist so the generic core stays
    `Python`_-free, not so `Python`_ callers ever need to supply them themselves; this binding
    builds one internally and never exposes it as a constructor parameter.
 @endrst
 */
using PyIfTemplate = AGRC::IfTemplate<std::string, std::string>;


void initCppIfTemplate(pybind11::module_ &m);

#endif
