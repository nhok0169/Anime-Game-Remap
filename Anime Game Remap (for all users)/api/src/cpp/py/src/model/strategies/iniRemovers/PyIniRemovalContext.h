#ifndef AGRemapPyBind_PyIniRemovalContext_H
#define AGRemapPyBind_PyIniRemovalContext_H

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

#include "AGRemapCore/model/strategies/iniRemovers/IniRemovalContext.h"


/**
 * @brief
 @rst
 Registers :cpp:class:`AGRemapCore::IniRemovalContext` with `Python`_ :raw-html:`<br />`
 :raw-html:`<br />`

 Bound straight, with no ``Py``-prefixed subclass in between: the struct is not a template, holds no
 `Python`_ state, and has nothing a `Python`_ caller could want that it does not already have
 @endrst
 *
 * @param m The module to register into
 */
void initCppIniRemovalContext(pybind11::module_ &m);

#endif
