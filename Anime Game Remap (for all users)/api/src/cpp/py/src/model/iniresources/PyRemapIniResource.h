#ifndef AGRemapPyBind_PyRemapIniResource_H
#define AGRemapPyBind_PyRemapIniResource_H

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

void initCppRemapIniResourceMixin(pybind11::module_ &m);
void initCppRemapIniResource(pybind11::module_ &m);
void initCppRemapIniFixResource(pybind11::module_ &m);
// initCppRemapIniGroupedResource moved to PyRemapIniGroupedResource.h
void initCppRemapIniDownload(pybind11::module_ &m);

#endif
