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
                                             bool negativeIndex, double claimShare, std::size_t overlapRings) {
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
    spec.claimShare = claimShare;
    spec.overlapRings = overlapRings;
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
    one of ``remap``'s groups -- the reverse remap turned around. On a cut component they are
    stand-ins: a kept vertex's weight on another component's group goes to the stand-in bone instead
    of being dropped; they never decide which component takes a triangle :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

negativeIndex: :class:`bool`
    ``True``: the negative-index strategy (the component draws the whole mod, other components'
    bones become the ``-index-1`` sentinel, its index buffers are trimmed to fully-live triangles).
    ``False``: the graph cut (the component takes the triangles the negative-index components leave,
    shared out among the cut components by majority, every buffer filtered to the vertices it uses)
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

claimShare: :class:`float`
    For a cut component, the least share of a vertex's weight on ``remap``'s groups for it to claim the
    vertex (``0`` to ``1``); a vertex below it goes to the next component that claims it, and one no
    component can claim falls back to the plain majority :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``0``, the plain majority

overlapRings: :class:`int`
    For a cut component, how many rings of its neighbours' triangles it draws as well, past its own
    edge, so a seam that opens when the skin poses is covered by the other side's copy. Ownership is
    unchanged :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``0``, no overlap
    )doc")
        .def(py::init([](const std::string &name, const py::object &remap, const py::object &secondary, bool negativeIndex,
                         double claimShare, std::size_t overlapRings) {
            return vgComponentSpecFromPy(name, remap, secondary, negativeIndex, claimShare, overlapRings);
        }), py::arg("name"), py::arg("remap") = py::none(), py::arg("secondary") = py::none(), py::arg("negativeIndex") = false,
            py::arg("claimShare") = 0.0, py::arg("overlapRings") = 0)
        .def_readwrite("splitGroups", &AGRC::VGComponentSpec::splitGroups, py::doc(R"doc(
Dict[:class:`int`, List[Tuple[:class:`int`, :class:`float`]]]: For a cut component, source groups whose weight is
SHARED among several of the component's bones, ``{source group: [(bone, share), ...]}``, applied over the vertex's
final weights; a vertex left with more than 4 influences keeps its 4 largest, renormalised. For a cloth part
between a bone it clips on and one it folds on. Empty by default
        )doc"))
        .def_readwrite("mirroredIbs", &AGRC::VGComponentSpec::mirroredIbs, py::doc(R"doc(
List[:class:`int`]: For a cut component, the source index buffers (by position) whose triangles get a MIRRORED
INNER LAYER: each corner copied once (flagged in :attr:`VGComponentBuffers.mirrored`) and each triangle followed
by its copy wound the other way, under the same source triangle id. For single-layer cloth whose back faces the
target's shader does not shade as cloth. Empty by default
        )doc"))
        .def_readwrite("mirrorBackedReach", &AGRC::VGComponentSpec::mirrorBackedReach, py::doc(R"doc(
:class:`float`: For a cut component with :attr:`mirroredIbs`: how far behind a mirrored triangle to look for a layer
of the mesh facing the other way, in model units; a triangle so backed gets no twin. Needs
:meth:`VGComponentSplit.setGeometry`. ``0`` (the default) mirrors every triangle
        )doc"))
        .def_readwrite("overlapRings", &AGRC::VGComponentSpec::overlapRings,
                       py::doc(":class:`int`: For a cut component, how many rings of its neighbours' triangles it draws as well"))
        .def_readwrite("claimShare", &AGRC::VGComponentSpec::claimShare,
                       py::doc(":class:`float`: For a cut component, the least own share of a vertex's weight to claim it"))
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
        .def_readonly("overlapTriangles", &AGRC::VGComponentSplitStats::overlapTriangles,
                      py::doc(":class:`int`: Cut only: triangles drawn as the overlap band -- see :attr:`VGComponentSpec.overlapRings`"))
        .def_readonly("sentinels", &AGRC::VGComponentSplitStats::sentinels, py::doc(":class:`int`: Negative index only: sentinel indices written"))
        .def_readonly("mirroredVertices", &AGRC::VGComponentSplitStats::mirroredVertices,
                      py::doc(":class:`int`: Cut only: vertices copied for the mirrored inner layer -- see :attr:`VGComponentSpec.mirroredIbs`"))
        .def_readonly("mirroredTriangles", &AGRC::VGComponentSplitStats::mirroredTriangles,
                      py::doc(":class:`int`: Cut only: triangles added as the mirrored inner layer"))
        .def_readonly("mirrorBacked", &AGRC::VGComponentSplitStats::mirrorBacked,
                      py::doc(":class:`int`: Cut only: triangles of a mirrored buffer given no twin, being backed -- see :attr:`VGComponentSpec.mirrorBackedReach`"))
        .def_readonly("splitVertices", &AGRC::VGComponentSplitStats::splitVertices,
                      py::doc(":class:`int`: Cut only: vertices whose weight was shared -- see :attr:`VGComponentSpec.splitGroups`"));

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
        .def_readonly("keptTriangleIds", &AGRC::VGComponentBuffers::keptTriangleIds,
                      py::doc("List[List[:class:`int`]]: Per source index buffer, the SOURCE index of every triangle in :attr:`ibs`, ascending -- what a mod's own ``drawindexed`` ranges are remapped through"))
        .def_readonly("mirrored", &AGRC::VGComponentBuffers::mirrored,
                      py::doc("List[:class:`bool`]: Per entry of :attr:`vertices`, whether it is a copy for the mirrored inner layer (empty without one)"))
        .def_readonly("mirrorLimits", &AGRC::VGComponentBuffers::mirrorLimits,
                      py::doc("List[:class:`float`]: Per entry of :attr:`vertices`, for a mirrored copy the most it may move inward (half way to a lining behind it), ``-1`` for no limit; empty unless :attr:`VGComponentSpec.mirrorBackedReach` applied"))
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
        .def("setGeometry", &AGRC::VGComponentSplit::setGeometry, py::arg("positions"), py::arg("normals"), py::doc(R"doc(
Hands the split the mod's own positions and normals, per source vertex --- what
:attr:`VGComponentSpec.mirrorBackedReach` asks about

Parameters
----------
positions: List[Tuple[:class:`float`, :class:`float`, :class:`float`]]
    Per source vertex, its position

normals: List[Tuple[:class:`float`, :class:`float`, :class:`float`]]
    Per source vertex, its normal
        )doc"))
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
        .def_static("mirrorPositionLine", [](const py::bytes &line, float offset) {
            return toBytes(AGRC::VGComponentSplit::mirrorPositionLine(fromBytes(line), offset));
        }, py::arg("line"), py::arg("offset"), py::doc(R"doc(
A GIMI ``Position.buf`` line (position, normal, tangent) for the mirrored inner layer: the normal turned round and
the position moved ``offset`` model units against the original normal. A line shorter than the normal comes back
as it is

Parameters
----------
line: :class:`bytes`
    The source line

offset: :class:`float`
    How far inward, in model units

Returns
-------
:class:`bytes`
    The mirrored line
        )doc"))
        .def_static("keepLines", [](const py::bytes &src, std::size_t bytesPerLine, const std::vector<std::size_t> &lines) {
            return toBytes(AGRC::VGComponentSplit::keepLines(fromBytes(src), bytesPerLine, lines));
        }, py::arg("src"), py::arg("bytesPerLine"), py::arg("lines"),
           py::doc("Keeps only the given lines of a fixed-stride buffer, in the order given"));
}
