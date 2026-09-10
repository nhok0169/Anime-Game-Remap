#ifndef AGRemapPyBind_PyHashes_H
#define AGRemapPyBind_PyHashes_H

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
 The `pybind11`_ bound ``Hashes`` class -- unlike ``ModMappedAssets`` (a generic,
 reusable engine), this is one *specific*, pre-populated instance of it: its hash data
 (:cpp:func:`AGRemapCore::Data::getHashDataRows`) and non-version index names (``name``,
 ``type``) are baked in at construction, matching the pure-Python ``Hashes`` class's own
 constructor contract (``model/assets/Hashes.py``, now removed entirely -- see git history)
 exactly: a bare, optional ``map`` is the only real constructor argument :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    Bound directly as the bare ``Hashes`` -- there is no Python source file behind this class at
    all any more, not even a thin wrapper: ``ModMappedAssets`` already provides every
    method (:cpp:func:`toWildcardList`-backed argument normalization included, via its
    ``nonVersionIndexNames``); this class only supplies the data. See :cpp:class:`PyModDictAssets`'s
    own note on the "two outcomes for porting a class" convention this project follows -- this is
    a step further than either of those two outcomes (outcome 2, full replacement, but with no
    Python file involved at construction time either, since the data itself moved into C++ too)
 @endrst
 */
void initCppHashes(pybind11::module_ &m);

#endif
