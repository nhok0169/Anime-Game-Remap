#ifndef AGRemapPyBind_PyIniNamingTools_H
#define AGRemapPyBind_PyIniNamingTools_H

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
 Binds `AGRC::IniNamingTools` as ``CppIniNamingTools`` -- deliberately NOT as ``IniNamingTools``,
 which the still-pure-`Python`_ class of that name already occupies and disagrees with. See the
 warning in the binding itself
 @endrst
 */
void initCppIniNamingTools(pybind11::module_ &m);

#endif
