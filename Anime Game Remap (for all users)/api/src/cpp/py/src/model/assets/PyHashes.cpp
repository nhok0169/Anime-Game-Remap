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
        )doc"))

        .def_property_readonly_static("LiveSuffix", [](py::object) { return AGRC::Hashes::LiveSuffix; }, py::doc(R"doc(
:class:`str`: A hash type's later generation of the same role's hash: ``<type>Live``, the hash bound once a texture's
mips have streamed in
        )doc"))
        .def_property_readonly_static("OldSuffix", [](py::object) { return AGRC::Hashes::OldSuffix; }, py::doc(R"doc(
:class:`str`: A hash type's older generations: ``<type>Old``, ``<type>Old2``, ...
        )doc"))

        .def_static("generationOf", &AGRC::Hashes::generationOf, py::arg("hashType"), py::doc(R"doc(
The hash type a generation of it is filed as, and which generation: ``("bangsDiffuse", 0)`` for ``"bangsDiffuse"``,
``1`` for its :attr:`LiveSuffix` (``"bangsDiffuseLive"``), ``2``, ``3``, ... for its :attr:`OldSuffix`, ``Old2``, ... --
so every hash a texture has had can be read as the one type

Parameters
----------
hashType: :class:`str`
    The hash type, as the table files it

Returns
-------
Tuple[:class:`str`, :class:`int`]
    The base type and the generation
)doc"))
        .def("age", &AGRC::Hashes::age, py::arg("name"), py::arg("hash"), py::doc(R"doc(
How old 'hash' is among 'name''s hashes, lower newer: the versions this table files 'name' at that are newer than the
one filing 'hash' (100 each), then the generation its hash type names -- ``0`` for a type's current hash, ``1`` for its
:attr:`LiveSuffix`, ``2``, ``3``, ... for its :attr:`OldSuffix`, ``Old2``, ... The lowest over every row filing 'hash'
(case-insensitively); ``1 << 20`` when none does

Parameters
----------
name: :class:`str`
    The mod name the hashes are filed under

hash: :class:`str`
    The hash

Returns
-------
:class:`int`
    The age
        )doc"));
}
