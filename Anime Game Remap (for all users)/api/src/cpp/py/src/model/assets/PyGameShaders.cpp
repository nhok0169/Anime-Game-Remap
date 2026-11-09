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

#include "PyGameShaders.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/GameShaders.h"


void initCppGameShaders(pybind11::module_ &m) {
    py::class_<AGRC::GameShaders, CoreModMappedAssets, py::smart_holder>(m, "GameShaders", R"doc(
This class inherits from :class:`ModMappedAssets`

The game pixel shaders a fix draws a character's parts with, per game version, pre-populated with this project's real data

The ``component`` is ``""`` and the ``type`` names the shader (``LynaeUpper``): its value is the shader's
decompiled HLSL; ``<shader>.<register>`` (eg. ``LynaeUpper.ps-cb4``) is a constant buffer its draw read at that
register, as 32-bit words in hex, comma-separated :raw-html:`<br />` :raw-html:`<br />`

Game shaders change on updates faster than anything else a remap reads, so each is filed under the game
version it was read from

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
            return std::make_unique<AGRC::GameShaders>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to

    **Default**: ``None``
        )doc"));
}
