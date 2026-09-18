#ifndef AGRemapPyBind_PyIniRemoveBuilder_H
#define AGRemapPyBind_PyIniRemoveBuilder_H

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

#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"


/**
 * @brief
 @rst
 Registers ``IniRemoveBuilder`` and its opaque ``CppIniRemoveBuilderArgs`` lookup table
 @endrst
 *
 * @param m The module to register into
 */
void initCppIniRemoveBuilder(pybind11::module_ &m);

#endif
