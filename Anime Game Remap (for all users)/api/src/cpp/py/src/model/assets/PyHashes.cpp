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

#include "PyHashes.h"

#include <utility>

#include "PyModMappedAssets.h"

#include "AGRemapCore/model/assets/Hashes.h"


void initCppHashes(pybind11::module_ &m) {
    py::class_<AGRC::Hashes, CoreModMappedAssets, py::smart_holder>(m, "Hashes", R"doc(
This class inherits from :class:`ModMappedAssets`

Class for managing hashes for a mod, pre-populated with this project's real hash data

:raw-html:`<br />`

.. note::
    Names of the available indices used for querying with the ``get``/``hasFrom``/``getKey``/
    ``replace``/``replaceAll`` methods (inherited from :class:`ModMappedAssets`) are:

    * version (version index)
    * name
    * type
    )doc")

        // Constructs the CORE AGRemapCore::Hashes, which builds the same table from the same
        // AGRemapCore::Data rows this file used to rebuild by hand, and sets its own
        // nonVersionIndexNames. That duplicate builder is gone.
        .def(py::init([](const py::object &map) {
            return std::make_unique<AGRC::Hashes>(map.is_none() ? PyObjectMap{} : convertMap(map.cast<py::dict>()));
        }), py::arg("map") = py::none(), py::doc(R"doc(
Constructs a new, fully-populated hash lookup table

Parameters
----------
map: Optional[Dict[Any, List[Any]]]
    The `adjacency list`_ that maps the hashes to fix from to the hashes to fix to using the
    predefined mods

    **Default**: ``None``
        )doc"));
}
