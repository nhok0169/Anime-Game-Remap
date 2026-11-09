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

#include "PyVGRemap.h"

#include <optional>
#include <unordered_map>

#include <pybind11/stl.h>

#include "AGRemapCore/model/VGRemap.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppVGRemap(pybind11::module_ &m) {
    // Full replacement of the pure-Python original (api/src/py/FixRaidenBoss2/model/VGRemap.py),
    // now deleted -- registered under the bare name directly. Real production data
    // (data/VGRemapData.py) constructs many of these via plain dict literals, which convert
    // automatically through pybind11's map caster -- no call-site changes needed there beyond the
    // import path.
    py::class_<AGRC::VGRemap, py::smart_holder>(m, "VGRemap", R"doc(
Class for handling the vertex group remaps for mods
    )doc")

        .def(py::init<std::unordered_map<long long, long long>>(), py::arg("vgRemap") = std::unordered_map<long long, long long>{}, py::doc(R"doc(
Constructs a new vertex group remap

Parameters
----------
vgRemap: Dict[:class:`int`, :class:`int`]
    The vertex group remap from one type of mod to another. **Default**: ``{}``
        )doc"))

        .def_property("remap", &AGRC::VGRemap::getRemap, &AGRC::VGRemap::setRemap, py::doc(R"doc(
Dict[:class:`int`, :class:`int`]: The vertex group remap
        )doc"))

        .def_property_readonly("maxIndex", &AGRC::VGRemap::getMaxIndex, py::doc(R"doc(
Optional[:class:`int`]: The maximum index in the vertex group remap, or ``None`` if :attr:`remap` is empty
        )doc"))

        .def("compact", [](const AGRC::VGRemap &self, std::size_t size) {
            AGRC::VGRemap::Compact compact = self.compact(size);
            return py::make_tuple(AGRC::VGRemap(compact.localRow), compact.forward, compact.reverse);
        }, py::arg("size"), py::doc(R"doc(
This row renumbered into a table of 'size' entries: every distinct target bone below 'size' it names gets a LOCAL id,
in ascending order of the bone -- so a buffer can hold local ids no wider than the table, with the table to turn them
back

Parameters
----------
size: :class:`int`
    The entries of the table

Returns
-------
Tuple[:class:`VGRemap`, List[:class:`int`], List[:class:`int`]]
    The row in local ids (a target past the table left out); the forward table, local id -> target bone (``0`` past the
    last); and the reverse table, target bone -> local id (``0`` for a bone the row does not name)
        )doc"));
}
