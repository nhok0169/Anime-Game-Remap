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

#include "PyTextureDescs.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/TextureDescs.h"


void initCppTextureDescs(pybind11::module_ &m) {
    py::class_<AGRC::TextureDescs, CoreModMappedAssets, py::smart_holder>(m, "TextureDescs", R"doc(
This class inherits from :class:`ModMappedAssets`

How the game creates a character's game textures, per game version, pre-populated with this project's real data

The ``component`` is ``""`` and the ``type`` is the texture's hash; the value is its description as a frame
dump's ``.dsc`` gives it, ``format,width,height,mips`` (``BC7_TYPELESS,2048,2048,12``) -- what a texture's own
file cannot say once a copy has dropped its mips. See :meth:`ModType.getTextureDesc` and
:meth:`RabbitFX.tagOf`

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
            return std::make_unique<AGRC::TextureDescs>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to

    **Default**: ``None``
        )doc"));
}
