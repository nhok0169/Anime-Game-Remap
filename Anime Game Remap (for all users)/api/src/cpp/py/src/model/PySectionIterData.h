#ifndef AGRemapPyBind_PySectionIterData_H
#define AGRemapPyBind_PySectionIterData_H

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

#include "iftemplate/PyIfContentPart.h"  // reuses PyIfContentPart/PyObjectHash/PyObjectEqual
#include "iftemplate/PyIfTemplate.h"
#include "AGRemapCore/model/SectionIterData.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief The `pybind11`_-facing names for `AGRC::SectionIterData`/`AGRC::SectionIterQueryData`\\<py::object, py::object\\>. Plain aliases, not subclasses.
 */
using PySectionIterData = AGRC::SectionIterData<std::string, std::string>;
using PySectionIterQueryData = AGRC::SectionIterQueryData<std::string, std::string>;


void initCppSectionIterData(pybind11::module_ &m);

#endif
