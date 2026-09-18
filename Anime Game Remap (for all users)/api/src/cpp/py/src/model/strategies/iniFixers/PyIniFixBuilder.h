#ifndef AGRemapPyBind_PyIniFixBuilder_H
#define AGRemapPyBind_PyIniFixBuilder_H

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

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


/**
 * @brief
 @rst
 Registers ``IniFixBuilder`` and its opaque ``CppIniFixBuilderArgs`` lookup table
 @endrst
 *
 * @param m The module to register into
 */
/**
 * @brief
 @rst
 Wraps a `Python`_ callable as an :cpp:type:`AGRemapCore::IniFixBuilder::Factory` -- see
 :cpp:func:`parseFactoryFromPy` for the two rules it exists to keep in one place
 @endrst
 *
 * @param factory The Python callable, taking ``(parser, toModName, modTypeId)``
 *
 * @return The wrapped factory
 */
AGRemapCore::IniFixBuilder::Factory fixFactoryFromPy(pybind11::object factory);

void initCppIniFixBuilder(pybind11::module_ &m);

#endif
