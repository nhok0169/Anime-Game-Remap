#ifndef AGRemapPyBind_PyGraphRename_H
#define AGRemapPyBind_PyGraphRename_H

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

#include "PyBaseIniGraphEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphRename.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `Python`_ ``GraphRename`` is :cpp:class:`AGRemapCore::GraphRename` itself: its arguments are parsed into the C++ members
 when it is built or they are set, and read back from them -- so one built in `Python`_ runs in a C++ fixer's list, and
 one built in C++ reads in `Python`_ as a ``GraphRename``
 @endrst
 */
using PyGraphRename = AGRC::GraphRename<std::string, std::string>;


/**
 * @brief Registers the Python-facing ``GraphRename``
 *
 * @param m The module to register into
 */
void initCppGraphRename(pybind11::module_ &m);

#endif
