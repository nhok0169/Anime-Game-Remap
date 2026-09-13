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

#include "PyVGComponentSplit.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/model/VGRemap.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;

namespace {
py::bytes toBytes(const AGRC::ByteVec &bytes) {
    return py::bytes(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

AGRC::ByteVec fromBytes(const py::bytes &bytes) {
    std::string s = bytes;
    return AGRC::ByteVec(s.begin(), s.end());
}
}


AGRC::VGComponentSpec vgComponentSpecFromPy(const std::string &name, const py::object &remap, const py::object &secondary,
                                             bool negativeIndex) {
    AGRC::VGComponentSpec spec;
    spec.name = name;
    if (py::isinstance<AGRC::VGRemap>(remap)) {
        spec.remap = remap.cast<AGRC::VGRemap>();
    } else if (!remap.is_none()) {
        spec.remap = AGRC::VGRemap(remap.cast<std::unordered_map<long long, long long>>());
    }
    if (!secondary.is_none()) {
        spec.secondary = secondary.cast<std::unordered_map<long long, long long>>();
    }
    spec.negativeIndex = negativeIndex;
    return spec;
}


void initCppVGComponentSplit(pybind11::module_ &m) {
    py::class_<AGRC::VGComponentSpec>(m, "VGComponentSpec", R"doc(
One target component of a skin made of several -- YelanTranquil's ``Body``, ``Bang`` and ``Eye`` --
and how a mod's vertex groups reach its bones

Parameters
----------
name: :class:`str`
    The component's name

remap: Union[:class:`VGRemap`, Dict[:class:`int`, :class:`int`]]
    The mod's vertex group (source index) to this component's bone

secondary: Optional[Dict[:class:`int`, :class:`int`]]
    Further source groups the component has a bone for, honoured only on a vertex that also carries
    one of ``remap``'s groups -- the reverse remap turned around. Only used by a negative-index
    component :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

negativeIndex: :class:`bool`
    ``True``: the negative-index strategy (the component draws the whole mod, other components'
    bones become the ``-index-1`` sentinel, its index buffers are trimmed to fully-live triangles).
    ``False``: the graph cut (the component takes the triangles the negative-index components leave,
    shared out among the cut components by majority, every buffer filtered to the vertices it uses)
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc")
        .def(py::init([](const std::string &name, const py::object &remap, const py::object &secondary, bool negativeIndex) {
            return vgComponentSpecFromPy(name, remap, secondary, negativeIndex);
        }), py::arg("name"), py::arg("remap") = py::none(), py::arg("secondary") = py::none(), py::arg("negativeIndex") = false)
        .def_readwrite("name", &AGRC::VGComponentSpec::name, py::doc(":class:`str`: The component's name"))
        .def_readwrite("remap", &AGRC::VGComponentSpec::remap, py::doc(":class:`VGRemap`: The mod's vertex group to this component's bone"))
        .def_readwrite("secondary", &AGRC::VGComponentSpec::secondary,
                       py::doc("Dict[:class:`int`, :class:`int`]: Further source groups, honoured only on an anchored vertex"))
        .def_readwrite("negativeIndex", &AGRC::VGComponentSpec::negativeIndex,
                       py::doc(":class:`bool`: Whether this is a negative-index component rather than a cut one"));

    py::class_<AGRC::VGComponentSplitStats>(m, "VGComponentSplitStats", R"doc(
Counts worth reporting about one component's split
    )doc")
        .def_readonly("vertexCount", &AGRC::VGComponentSplitStats::vertexCount, py::doc(":class:`int`: The mod's vertices"))
        .def_readonly("keptVertices", &AGRC::VGComponentSplitStats::keptVertices, py::doc(":class:`int`: The vertices the component draws"))
        .def_readonly("trianglesKept", &AGRC::VGComponentSplitStats::trianglesKept, py::doc("List[:class:`int`]: Per index buffer, the triangles kept"))
        .def_readonly("trianglesDropped", &AGRC::VGComponentSplitStats::trianglesDropped, py::doc("List[:class:`int`]: Per index buffer, the triangles left to the others"))
        .def_readonly("renormalised", &AGRC::VGComponentSplitStats::renormalised, py::doc(":class:`int`: Cut only: vertices that lost foreign weight"))
        .def_readonly("neighbourSkinned", &AGRC::VGComponentSplitStats::neighbourSkinned, py::doc(":class:`int`: Cut only: vertices skinned to a neighbour's bone"))
        .def_readonly("sentinels", &AGRC::VGComponentSplitStats::sentinels, py::doc(":class:`int`: Negative index only: sentinel indices written"));

    py::class_<AGRC::VGComponentBuffers>(m, "VGComponentBuffers", R"doc(
What one component gets out of a split: the vertices it draws and its buffers over them
    )doc")
        .def_readonly("vertices", &AGRC::VGComponentBuffers::vertices,
                      py::doc("List[:class:`int`]: The mod's vertex indices this component draws, ascending (every one for a negative-index component)"))
        .def_readonly("weights", &AGRC::VGComponentBuffers::weights,
                      py::doc("List[List[:class:`float`]]: Per kept vertex, its 4 weights in the component's bones"))
        .def_readonly("indices", &AGRC::VGComponentBuffers::indices,
                      py::doc("List[List[:class:`int`]]: Per kept vertex, its 4 bone indices in the component's numbering (negative sentinels for negative index)"))
        .def_readonly("ibs", &AGRC::VGComponentBuffers::ibs,
                      py::doc("List[List[List[:class:`int`]]]: Per source index buffer, the triangles this component draws -- renumbered into ``vertices`` for a cut, in the mod's numbering for negative index"))
        .def_readonly("live", &AGRC::VGComponentBuffers::live, py::doc("List[:class:`bool`]: Negative index only: per mod vertex, whether it carries no sentinel"))
        .def_readonly("stats", &AGRC::VGComponentBuffers::stats, py::doc(":class:`VGComponentSplitStats`: Counts worth reporting"));

    py::class_<AGRC::VGComponentSplit>(m, "VGComponentSplit", R"doc(
Splits one mod's geometry across the components of a target skin

The ``Blend.buf`` decides which component each vertex belongs to, the index buffers decide which
triangles go where, and every vertex buffer is then filtered to the vertices a component keeps -- so
the buffers of a mod cannot be split one at a time, which is what makes this a grouped resource's job
(see :class:`VGSplitGroupResource`). The strategies mirror ``Tools/VGRemapFinder``'s
``ComponentSplit.py`` (its ``fill`` mode): the negative-index components first draw every triangle
all of whose corners are live in them, and the cut components share the rest out by majority

Parameters
----------
weights: List[List[:class:`float`]]
    Per vertex, its 4 blend weights

indices: List[List[:class:`int`]]
    Per vertex, its 4 vertex group indices

ibs: List[List[List[:class:`int`]]]
    The mod's index buffers, one per drawn object, each a list of ``[a, b, c]`` triangles

specs: List[:class:`VGComponentSpec`]
    Every component of the target
    )doc")
        .def(py::init<AGRC::VGComponentSplit::Weights, AGRC::VGComponentSplit::Indices,
                      std::vector<AGRC::VGComponentSplit::Triangles>, std::vector<AGRC::VGComponentSpec>>(),
             py::arg("weights"), py::arg("indices"), py::arg("ibs"), py::arg("specs"))
        .def_property_readonly("vertexCount", &AGRC::VGComponentSplit::vertexCount, py::doc(":class:`int`: The mod's vertices"))
        .def("split", &AGRC::VGComponentSplit::split, py::arg("component"), py::doc(R"doc(
Splits for one component

Parameters
----------
component: :class:`str`
    The component's name

Returns
-------
:class:`VGComponentBuffers`
    The component's vertices and buffers
        )doc"))
        .def_static("encodeBlend", [](const AGRC::VGComponentSplit::Weights &weights, const AGRC::VGComponentSplit::Indices &indices) {
            return toBytes(AGRC::VGComponentSplit::encodeBlend(weights, indices));
        }, py::arg("weights"), py::arg("indices"), py::doc("Encodes weights and indices into ``Blend.buf`` bytes (4 floats then 4 signed ints per line)"))
        .def_static("encodeIb", [](const AGRC::VGComponentSplit::Triangles &triangles) {
            return toBytes(AGRC::VGComponentSplit::encodeIb(triangles));
        }, py::arg("triangles"), py::doc("Encodes triangles into ``.ib`` bytes (3 unsigned ints per triangle)"))
        .def_static("keepLines", [](const py::bytes &src, std::size_t bytesPerLine, const std::vector<std::size_t> &lines) {
            return toBytes(AGRC::VGComponentSplit::keepLines(fromBytes(src), bytesPerLine, lines));
        }, py::arg("src"), py::arg("bytesPerLine"), py::arg("lines"),
           py::doc("Keeps only the given lines of a fixed-stride buffer, in the order given"));
}
