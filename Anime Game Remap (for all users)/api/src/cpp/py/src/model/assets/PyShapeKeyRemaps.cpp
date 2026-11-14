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

#include "PyShapeKeyRemaps.h"

#include <optional>
#include <string>

#include <pybind11/stl.h>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ShapeKeyRemaps.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppShapeKeyRemaps(pybind11::module_ &m) {
    py::class_<AGRC::ShapeKeyRemaps, py::smart_holder>(m, "ShapeKeyRemaps", R"doc(
The order a mod's shape keys are put in when it is remapped onto another character, pre-populated with this project's
real data

The game drives a character's shape keys (blinks, mouth shapes) by SLOT, and two characters can number the same keys
differently, so a fix writes the mod's shape-key buffers again in the target's order (:meth:`WWMIShapeKeys.reorder`).
The value is that order: for each of the target's slots, the source key it takes (``-1`` for none), comma-separated --
see :meth:`ModType.getShapeKeyRemap`

.. note::
    Six index columns with two versions, as :class:`VGRemaps` has: ``fromVersion``, ``fromChar``, ``fromComp``,
    ``toVersion``, ``toChar``, ``toComp``. Both component columns are ``""`` on every shipped row
    )doc")
        .def(py::init<>(), py::doc("Constructs a new, fully-populated table"))
        .def("get", [](const AGRC::ShapeKeyRemaps &self, const std::string &fromChar, const std::string &toChar,
                       const std::optional<AGRC::Version> &fromVersion, const std::optional<AGRC::Version> &toVersion) {
            return self.get({fromChar, std::string(""), toChar, std::string("")}, {fromVersion, toVersion}, false);
        }, py::arg("fromChar"), py::arg("toChar"), py::arg("fromVersion") = py::none(), py::arg("toVersion") = py::none(), py::doc(R"doc(
The order from one character's version onto another's, as written in the table -- each version the newest row at or
below it

Parameters
----------
fromChar: :class:`str`
    The character remapped from

toChar: :class:`str`
    The character remapped to

fromVersion: Optional[:class:`Version`]
    **Default**: ``None``, the latest

toVersion: Optional[:class:`Version`]
    **Default**: ``None``, the latest

Returns
-------
Optional[:class:`str`]
    The order, comma-separated, or ``None`` when the table has none for the pair
        )doc"))
        .def("__len__", &AGRC::ShapeKeyRemaps::size, py::doc("The number of rows"));
}
