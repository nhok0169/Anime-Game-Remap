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

#include "PyMaterialCodes.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/MaterialCodes.h"


void initCppMaterialCodes(pybind11::module_ &m) {
    py::class_<AGRC::MaterialCodes, CoreModMappedAssets, py::smart_holder>(m, "MaterialCodes", R"doc(
This class inherits from :class:`ModMappedAssets`

What the codes of a character's single-channel code maps mean, per game version, pre-populated with this project's real data

The ``component`` is the draw slot (``component6``) and the ``type`` names a code map role and what of it:
``<role>.<field>`` -- the bits a field of the code occupies (``propsDetail.flags.film``: ``0x10``;
``propsDetail.material``: ``0x0f``); ``<role>.<field>.<name>`` -- one code of a field (``pouchDetail.material.plain``:
``15``). Values are numbers, decimal or ``0x`` hex -- see :meth:`ModType.getMaterialCode` :raw-html:`<br />` :raw-html:`<br />`

A game update that renumbers a character's materials is new rows at the new version, not an edit to a filter

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
            return std::make_unique<AGRC::MaterialCodes>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to

    **Default**: ``None``
        )doc"));
}
