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
        )doc"));
}
