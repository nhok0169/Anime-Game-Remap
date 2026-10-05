#ifndef AGRemapPyBind_PyIniParseBuilder_H
#define AGRemapPyBind_PyIniParseBuilder_H

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

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


/**
 * @brief
 @rst
 Registers ``IniParseBuilder`` and its opaque ``CppIniParseBuilderArgs`` lookup table
 @endrst
 *
 * @param m The module to register into
 */
/**
 * @brief
 @rst
 Wraps a `Python`_ callable as an :cpp:type:`AGRemapCore::IniParseBuilder::Factory` :raw-html:`<br />`
 :raw-html:`<br />`

 Takes the GIL (the core calls a factory from plain C++, where it is not held) and returns the
 result through ``holdPyStrategy``, which is what keeps a `Python`_ subclass's identity instead of
 slicing it away. Shared with :cpp:class:`AGRemapCore::StrategyOverrides`' binding so there is one
 implementation of both of those rules
 @endrst
 *
 * @param factory The Python callable, taking ``(iniFile, modTypeId)``
 *
 * @return The wrapped factory
 */
AGRemapCore::IniParseBuilder::Factory parseFactoryFromPy(pybind11::object factory);

void initCppIniParseBuilder(pybind11::module_ &m);

#endif
