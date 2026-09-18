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

#ifndef AGRemapPyBind_PyVGComponentMerge_H
#define AGRemapPyBind_PyVGComponentMerge_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/buffers/VGComponentMerge.h"

/**
 * @brief Builds a :cpp:class:`AGRemapCore::VGMergeComponentSpec` from what Python hands over -- a ``VGRemap`` or a plain ``dict`` for the remap
 */
AGRemapCore::VGMergeComponentSpec vgMergeComponentSpecFromPy(const std::string &name, const pybind11::object &remap);

void initCppVGComponentMerge(pybind11::module_ &m);

#endif
