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

#ifndef PyGraphVarRename_H
#define PyGraphVarRename_H

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/GraphVarRename.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief Binds `AGRC::GraphVarRename`
 * @param m The module to bind it to
 */
void initCppGraphVarRename(pybind11::module_ &m);

#endif
