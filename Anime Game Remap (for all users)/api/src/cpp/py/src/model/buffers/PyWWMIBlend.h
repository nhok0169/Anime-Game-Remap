#ifndef AGRemapPyBind_PyWWMIBlend_H
#define AGRemapPyBind_PyWWMIBlend_H

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

void initCppWWMIBlend(pybind11::module_ &m);
void initCppWWMIShapeKeys(pybind11::module_ &m);
void initCppRabbitFX(pybind11::module_ &m);
void initCppWWMIRuntime(pybind11::module_ &m);
void initCppDrawUVCoverage(pybind11::module_ &m);

#endif
