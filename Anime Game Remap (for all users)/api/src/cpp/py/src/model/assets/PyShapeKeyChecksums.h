#ifndef AGRemapPyBind_PyShapeKeyChecksums_H
#define AGRemapPyBind_PyShapeKeyChecksums_H

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
 * @brief
 @rst
 The `pybind11`_ bound ``ShapeKeyChecksums`` class -- the exact same pattern as ``Indices`` (see
 :cpp:class:`PyIndices`'s note): one *specific*, pre-populated ``ModMappedAssets`` with its data
 (:cpp:func:`AGRemapCore::Data::getShapeKeyChecksumDataRows`) and its non-version index names
 (``name``, ``component``, ``type``) baked in at construction; a bare, optional ``map`` is the only
 constructor argument
 @endrst
 */
void initCppShapeKeyChecksums(pybind11::module_ &m);

#endif
