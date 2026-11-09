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

#include "PyDrawPasses.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/DrawPasses.h"


void initCppDrawPasses(pybind11::module_ &m) {
    py::class_<AGRC::DrawPasses, CoreModMappedAssets, py::smart_holder>(m, "DrawPasses", R"doc(
This class inherits from :class:`ModMappedAssets`

The draw passes of a WWMI character's draw slots, per game version, pre-populated with this project's real data

The slot is the ``component`` (``component0``, ``component1``, ...) and the ``type`` names a pass and
what is read of it: ``<pass>`` -- the vertex shaders the pass draws with, comma-separated; ``<pass>.ps`` -- its
pixel shader; ``<pass>.tags`` -- the ``filter_index`` a fix tags each of those vertex shaders with, in the same
order. The passes are ``main`` (``main2``, ... for a slot drawn more than once) and ``other`` (an outline, a
second pass) :raw-html:`<br />` :raw-html:`<br />`

A fix gates what it binds on its target slot's passes. A game update that changes a pass's shader is a new
row at the new version; a fix asked for an older version still reads that version's rows

:raw-html:`<br />`

.. note::
    Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/``getKeys``/
    ``versionOf``/``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:

    * version (version index)
    * name
    * component
    * type
    )doc")
        .def(py::init([](const py::object &map) {
            return std::make_unique<AGRC::DrawPasses>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to

    **Default**: ``None``
        )doc"));
}
