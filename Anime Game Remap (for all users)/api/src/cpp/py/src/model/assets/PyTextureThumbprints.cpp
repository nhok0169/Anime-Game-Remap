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

#include "PyTextureThumbprints.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/TextureThumbprints.h"


void initCppTextureThumbprints(pybind11::module_ &m) {
    py::class_<AGRC::TextureThumbprints, CoreModMappedAssets, py::smart_holder>(m, "TextureThumbprints", R"doc(
This class inherits from :class:`ModMappedAssets`

What a character's game textures look like, as small thumbprints, per game version, pre-populated with this project's real data

The ``component`` is ``""`` and the ``type`` is the texture's hash; the value is its thumbprint -- a 16 x 16
grayscale box average of the decoded texture, each pixel's ``(r + g + b) / 3`` -- as 256 bytes in hex, row by row.
A fix places a mod file that carries no hash by correlating its own thumbprint against these. See
:meth:`ModType.getTextureThumbprints` and :cpp:member:`WWMITextureFacts::textureThumbprints`

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
            return std::make_unique<AGRC::TextureThumbprints>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to

    **Default**: ``None``
        )doc"));
}
