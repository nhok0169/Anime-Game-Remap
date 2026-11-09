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

#include "PyMaterialColours.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/MaterialColours.h"


void initCppMaterialColours(pybind11::module_ &m) {
    py::class_<AGRC::MaterialColours, CoreModMappedAssets, py::smart_holder>(m, "MaterialColours", R"doc(
This class inherits from :class:`ModMappedAssets`

The colours a character's multi-channel material masks mark each material with, per game version, pre-populated with this project's real data

The ``component`` is the draw slot (``component3``) and the ``type`` names a mask role and a material
(``torsoMask.skin``); the value is the :class:`CppColour` the mask marks it with, written by
:meth:`valueOf` and read by :meth:`ModType.getMaterialColour` :raw-html:`<br />` :raw-html:`<br />`

A colour every character shares (a flat normal map, white) is a :class:`Colours` constant instead, which a
row may be written with

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
            return std::make_unique<AGRC::MaterialColours>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to

    **Default**: ``None``
        )doc"))
        .def_static("valueOf", &AGRC::MaterialColours::valueOf, py::arg("colour"), py::doc(R"doc(
A colour as the table holds it: ``r,g,b,a``

Parameters
----------
colour: :class:`CppColour`
    The colour

Returns
-------
:class:`str`
    The value
        )doc"))
        .def_static("colourOf", &AGRC::MaterialColours::colourOf, py::arg("value"), py::doc(R"doc(
The colour a value of the table holds (:meth:`valueOf` read back)

Parameters
----------
value: :class:`str`
    The value, ``r,g,b,a``

Raises
------
:class:`ValueError`
    When the value is not four comma-separated channels

Returns
-------
:class:`CppColour`
    The colour
        )doc"));
}
