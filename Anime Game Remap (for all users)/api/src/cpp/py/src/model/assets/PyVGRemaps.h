#ifndef AGRemapPyBind_PyVGRemaps_H
#define AGRemapPyBind_PyVGRemaps_H

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

#include "AGRemapCore/model/assets/VGRemaps.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_ bound ``VGRemaps`` -- one *specific*, pre-populated :cpp:class:`AGRC::ModAssets`
 (see :cpp:class:`AGRC::VGRemaps`), the same "the data is baked in" shape as
 :cpp:class:`PyHashes`/:cpp:class:`PyIndices`/:cpp:class:`PyVertexCounts` :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    Like :cpp:class:`PyVertexCounts`, and unlike ``Hashes``/``Indices``, this is **not** registered
    as a subclass of a bound base: the pybind-registered ``ModAssets`` is the
    ``<std::string, py::object>`` instantiation while this table's value type is
    :cpp:class:`AGRC::VGRemap`, so they are unrelated C++ types. Its methods are bound directly here

 .. note::
    Two version columns (``fromVersion``, ``toVersion``) is exactly why the core class is a
    :cpp:class:`ModAssets` (a linear scan) rather than a :cpp:class:`ModDictAssets` -- and why,
    unlike :cpp:class:`PyVertexCounts`, an unspecified non-version column here really is a
    wildcard rather than being filled in

 .. note::
    Replaces the pure-Python ``VGRemaps`` (``model/assets/VGRemaps.py``, deleted), which built
    itself from ``data/VGRemapData.py`` through a `pandas`_ ``DataFrame``; the C++ table is built
    from :cpp:func:`AGRC::Data::getVGRemapDataRows` with no pandas involved at all
 @endrst
 */
void initCppVGRemaps(pybind11::module_ &m);

#endif
