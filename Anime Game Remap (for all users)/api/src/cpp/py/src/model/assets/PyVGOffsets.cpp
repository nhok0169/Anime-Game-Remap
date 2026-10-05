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

#include "PyVGOffsets.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/VGOffsets.h"


void initCppVGOffsets(pybind11::module_ &m) {
    py::class_<AGRC::VGOffsets, CoreModMappedAssets, py::smart_holder>(m, "VGOffsets", R"doc(
This class inherits from :class:`ModMappedAssets`

Class for managing the ``vg_offset`` of a WWMI draw slot, pre-populated with this project's real data

:raw-html:`<br />`

.. note::
    Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/
    ``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:

    * version (version index)
    * name
    * component
    * type

    ``component`` is ``""`` on every row: a WWMI character's draw slots share ONE merged skeleton,
    so the slot is the ``type`` (``component0``, ``component1``, ...), exactly as
    :class:`Indices` files a WWMI ``match_first_index``
    )doc")
        // See PyIndices.cpp's identical note: the core class owns the table and its index names.
        .def(py::init([](const py::object &map) {
            return std::make_unique<AGRC::VGOffsets>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to using the
    predefined mods

    **Default**: ``None``
        )doc"));
}
