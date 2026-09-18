#ifndef AGRemapPyBind_PyIndices_H
#define AGRemapPyBind_PyIndices_H

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
 The `pybind11`_ bound ``Indices`` class -- the exact same pattern as ``Hashes`` (see
 its own class-level note): one *specific*, pre-populated instance of
 ``ModMappedAssets``, with its index data (:cpp:func:`AGRemapCore::Data::getIndexDataRows`)
 and non-version index names (``name``, ``component``, ``type``) baked in at construction,
 matching the pure-Python ``Indices`` class's own constructor contract (``model/assets/Indices.py``,
 now removed entirely -- see git history) exactly: a bare, optional ``map`` is the only real
 constructor argument
 @endrst
 */
void initCppIndices(pybind11::module_ &m);

#endif
