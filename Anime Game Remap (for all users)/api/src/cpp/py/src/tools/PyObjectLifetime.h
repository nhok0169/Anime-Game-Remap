#ifndef AGRemapPyBind_PyObjectLifetime_H
#define AGRemapPyBind_PyObjectLifetime_H

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


/**
 * @brief
 @rst
 Sets :cpp:class:`AGRemapCore::ObjectLifetime`'s hook for the module: when a section, a part or a graph is freed,
 every `Python`_ object that wraps it without owning it is invalidated -- taken out of `pybind11`_'s registry of
 instances and left pointing at nothing :raw-html:`<br />` :raw-html:`<br />`

 Without it, such a wrapper outlived its object whenever the object's owner (an :cpp:class:`AGRemapCore::IniFile`,
 a graph that copied its sections, a fixer) was freed before whoever borrowed it -- and `pybind11`_, which finds a
 wrapper by address, then handed that old wrapper back for the next object allocated at the same address. An
 invalidated wrapper raises if it is used, and is never handed out again. Must be called after the classes it
 watches are registered: it reads their `Python`_ types off 'm'
 @endrst
 *
 * @param m The module
 */
void initCppObjectLifetime(pybind11::module_ &m);

#endif
