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

#include "PyBoneCentroids.h"

#include <utility>

#include "PyModMappedAssets.h"
#include "AGRemapCore/model/assets/BoneCentroids.h"


void initCppBoneCentroids(pybind11::module_ &m) {
    py::class_<AGRC::BoneCentroids, CoreModMappedAssets, py::smart_holder>(m, "BoneCentroids", R"doc(
This class inherits from :class:`ModMappedAssets`

Each bone's rest-pose centroid on a character's own model, per game version, pre-populated with this project's real data

The ``component`` is ``""`` and the ``type`` is the bone's id in that version's numbering: its value is
the centroid, ``x,y,z``. :class:`SkeletonNumberingProbe` scores which numbering a mod's blend is in
against these -- see :meth:`ModType.getBoneCentroids`

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
            return std::make_unique<AGRC::BoneCentroids>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the values to fix from to the values to fix to

    **Default**: ``None``
        )doc"));
}
