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

#include "PyTextureKinds.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/TextureKinds.h"


void initCppTextureKinds(pybind11::module_ &m) {
    py::class_<AGRC::TextureKinds, CoreModMappedAssets, py::smart_holder>(m, "TextureKinds", R"doc(
This class inherits from :class:`ModMappedAssets`

What each of a character's texture roles IS, per game version, pre-populated with this project's real data

The ``component`` is the draw slot the role is bound on (``component3``), or ``""`` for a role no one slot owns; the
``type`` is the role (``upperDiffuse``); the value is its KIND (``Diffuse``), or ``""`` for a role with no kind (a game
texture such as ``ramp``). Two roles of one kind stand in for each other where a fix binds by kind -- see
:meth:`ModType.getTextureKind` and :attr:`WWMIFixerConfig.roleKinds`. Kinds are one vocabulary across characters

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
            return std::make_unique<AGRC::TextureKinds>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to

    **Default**: ``None``
        )doc"));
}
