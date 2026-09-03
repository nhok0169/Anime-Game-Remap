#ifndef AGRemapPyBind_PyModType_H
#define AGRemapPyBind_PyModType_H

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/ModType.h"


void initCppModType(pybind11::module_ &m);

// ModType::fixIni takes an IniFile, whose binding (IniFile) has to register AFTER CppModType --
// its own constructor names CppModType. pybind11 bakes a def()'s signature string at registration
// time, so declaring fixIni alongside the rest of CppModType would render its parameter as a raw
// C++ type name (see PyIfContentPartColour.cpp's note on what that does to core.pyi). This second
// pass runs once IniFile exists.
void initCppModTypeLateBindings(pybind11::module_ &m);

#endif
