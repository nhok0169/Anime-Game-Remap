#ifndef AGRemapPyBind_PyModType_H
#define AGRemapPyBind_PyModType_H

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
#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/ModType.h"


void initCppModType(pybind11::module_ &m);

// ModType::fixIni takes an IniFile, whose binding (IniFile) has to register AFTER ModType --
// its own constructor names ModType. pybind11 bakes a def()'s signature string at registration
// time, so declaring fixIni alongside the rest of ModType would render its parameter as a raw
// C++ type name (see PyIfContentPartColour.cpp's note on what that does to core.pyi). This second
// pass runs once IniFile exists.
void initCppModTypeLateBindings(pybind11::module_ &m);

#endif
