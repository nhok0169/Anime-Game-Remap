#ifndef AGRemapPyBind_PyShapeKeyDispatchSizes_H
#define AGRemapPyBind_PyShapeKeyDispatchSizes_H

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
#include "PyModMappedAssets.h"

namespace py = pybind11;

/**
 * @brief The `pybind11`_ bound ``ShapeKeyDispatchSizes`` class -- the pattern of ``ShapeKeyChecksums`` (see its note): a
 *      pre-populated ``ModMappedAssets`` with its data and index names baked in, an optional ``map`` its only argument
 */
void initCppShapeKeyDispatchSizes(pybind11::module_ &m);

#endif
