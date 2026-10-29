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

#include "PyWWMIBlend.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/data/RabbitFX.h"
#include "AGRemapCore/model/buffers/DrawUVCoverage.h"
#include "AGRemapCore/model/buffers/WWMIBlend.h"
#include "AGRemapCore/model/buffers/WWMIShapeKeys.h"
#include "AGRemapCore/model/buffers/SkeletonNumberingProbe.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;

namespace {
    py::list elementsToPy(std::vector<std::unique_ptr<AGRC::BufElementType>> elements) {
        py::list result;
        for (auto& element : elements) {
            result.append(py::cast(std::move(element)));
        }

        return result;
    }

    AGRC::VGRemap vgRemapFromPy(const py::object& row) {
        if (py::isinstance<AGRC::VGRemap>(row)) {
            return row.cast<AGRC::VGRemap>();
        }

        return AGRC::VGRemap(row.cast<std::unordered_map<long long, long long>>());
    }
}


void initCppWWMIBlend(pybind11::module_ &m) {
    py::class_<AGRC::WWMIBlend> wwmiBlend(m, "WWMIBlend", R"doc(
Remaps the vertex groups of a Wuthering Waves (WWMI) mod's ``Blend.buf``

A WWMI ``Blend.buf`` line is N one-byte bone ids followed by N one-byte weights, where N -- the
influences per vertex -- is a property of the character (four for Sanhua, eight for Chisa) and is read
off the buffer's size against its ``Position.buf`` (:meth:`layoutOf`) rather than assumed. Three shapes
of mod need more than :meth:`BlendFile.remap`:

* a character past 256 merged bones keeps each vertex's true 16-bit ids in a ``BlendRemapVertexVG.buf``
  beside the blend, whose 8-bit ids are then only an index into WWMI's blend remap --
  :meth:`remapThroughVertexVG`
* a mod from before WWMI's merged skeleton writes each component's OWN bone ids, which have to be lifted
  into the merged skeleton through that component's ``vg_map`` before the row applies -- :meth:`liftLegacy`
* a remap ONTO a character past 256 bones has to write WWMI's own blend remap -- :meth:`writeRemap`

and a target whose draws read a different number of influences than the mod carries is met by
:meth:`widen` and :meth:`narrow`

Every method raises a :class:`RuntimeError` with the reason when a buffer cannot be read or written in
the shape asked

.. note::
    Every method is static; the class holds nothing
    )doc");

    py::class_<AGRC::WWMIBlend::Layout>(wwmiBlend, "Layout", R"doc(
What a mod's ``Position.buf`` and ``Blend.buf`` sizes say its blend layout is
    )doc")
        .def(py::init<>())
        .def_readwrite("vertices", &AGRC::WWMIBlend::Layout::vertices, R"doc(
:class:`int`: How many vertices the mesh has
        )doc")
        .def_readwrite("influences", &AGRC::WWMIBlend::Layout::influences, R"doc(
:class:`int`: How many (id, weight) slots each vertex carries
        )doc");

    py::class_<AGRC::WWMIBlend::LegacyLift>(wwmiBlend, "LegacyLift", R"doc(
A mod from before WWMI's merged skeleton, as :meth:`WWMIBlend.liftLegacy` and :meth:`WWMIBlend.writeRemap`
read one

Parameters
----------
indexPath: :class:`str`
    The mod's index buffer

drawRanges: Dict[:class:`int`, List[Tuple[:class:`int`, :class:`int`]]]
    source component -> the (index count, first index) of each of its draws

vgMaps: Dict[:class:`int`, List[:class:`int`]]
    source component -> its ``vg_map``: component-local bone id -> merged bone id
    )doc")
        .def(py::init([](std::string indexPath, std::map<int, std::vector<std::pair<long long, long long>>> drawRanges,
                         std::map<int, std::vector<int>> vgMaps) {
            return AGRC::WWMIBlend::LegacyLift{std::move(indexPath), std::move(drawRanges), std::move(vgMaps)};
        }), py::arg("indexPath"), py::arg("drawRanges"), py::arg("vgMaps"))
        .def_readwrite("indexPath", &AGRC::WWMIBlend::LegacyLift::indexPath, ":class:`str`: The mod's index buffer")
        .def_readwrite("drawRanges", &AGRC::WWMIBlend::LegacyLift::drawRanges,
                       "Dict[:class:`int`, List[Tuple[:class:`int`, :class:`int`]]]: source component -> its draws")
        .def_readwrite("vgMaps", &AGRC::WWMIBlend::LegacyLift::vgMaps,
                       "Dict[:class:`int`, List[:class:`int`]]: source component -> its ``vg_map``");

    py::class_<AGRC::WWMIBlend::RemapFiles>(wwmiBlend, "RemapFiles", R"doc(
The three buffers of a WWMI blend remap, besides the remapped blend itself

Parameters
----------
vertexVG: :class:`str`
    Every vertex's remapped ids at their full 16 bits

forward: :class:`str`
    :attr:`WWMIBlend.RemapSize` uint16, local id -> merged bone

reverse: :class:`str`
    :attr:`WWMIBlend.RemapSize` uint16, merged bone -> local id
    )doc")
        .def(py::init([](std::string vertexVG, std::string forward, std::string reverse) {
            return AGRC::WWMIBlend::RemapFiles{std::move(vertexVG), std::move(forward), std::move(reverse)};
        }), py::arg("vertexVG"), py::arg("forward"), py::arg("reverse"))
        .def_readwrite("vertexVG", &AGRC::WWMIBlend::RemapFiles::vertexVG, ":class:`str`: The 16-bit ids")
        .def_readwrite("forward", &AGRC::WWMIBlend::RemapFiles::forward, ":class:`str`: local id -> merged bone")
        .def_readwrite("reverse", &AGRC::WWMIBlend::RemapFiles::reverse, ":class:`str`: merged bone -> local id");

    wwmiBlend.attr("PositionStride") = AGRC::WWMIBlend::PositionStride;
    wwmiBlend.attr("MaxByteBone") = AGRC::WWMIBlend::MaxByteBone;
    wwmiBlend.attr("RemapSize") = AGRC::WWMIBlend::RemapSize;

    wwmiBlend.def_static("blendElements", [](std::size_t influences) {
        return elementsToPy(AGRC::WWMIBlend::blendElements(influences));
    }, py::arg("influences"), R"doc(
The elements of a WWMI ``Blend.buf`` line: ``influences`` one-byte ids under ``"BLENDINDICES"``,
then ``influences`` one-byte weights under ``"BLENDWEIGHT"``

Parameters
----------
influences: :class:`int`
    How many (id, weight) slots each vertex carries

Returns
-------
List[:class:`BufElementType`]
    The elements
    )doc");

    wwmiBlend.def_static("vertexVGElements", [](std::size_t influences) {
        return elementsToPy(AGRC::WWMIBlend::vertexVGElements(influences));
    }, py::arg("influences"), R"doc(
The elements of a ``BlendRemapVertexVG.buf`` line: ``influences`` 16-bit ids under
``"BLENDINDICES"``, and no weights

Parameters
----------
influences: :class:`int`
    How many ids each vertex carries

Returns
-------
List[:class:`BufElementType`]
    The elements
    )doc");

    wwmiBlend.def_static("layoutOf", &AGRC::WWMIBlend::layoutOf, py::arg("blendPath"), py::arg("positionPath"), R"doc(
The vertex count and influences per vertex the two buffers' SIZES imply

Parameters
----------
blendPath: :class:`str`
    The mod's ``Blend.buf``

positionPath: :class:`str`
    Its ``Position.buf``, which gives the vertex count

Returns
-------
Optional[:class:`WWMIBlend.Layout`]
    The layout, or ``None`` when the sizes imply none: no vertices, a blend that does not divide by them,
    or an odd number of bytes per vertex
    )doc");

    wwmiBlend.def_static("influencesOf", &AGRC::WWMIBlend::influencesOf, py::arg("blendPath"), py::arg("vertices"),
                         py::arg("positionPath"), py::arg("fallback") = 4, R"doc(
How many influences a vertex of the mod's blend carries -- :meth:`layoutOf`'s answer, else the blend's size
over ``vertices``, else ``fallback``

Parameters
----------
blendPath: :class:`str`
    The mod's ``Blend.buf``

vertices: :class:`int`
    The vertex count the mod declares, used only when the ``Position.buf`` does not answer. Zero or less
    for none

positionPath: :class:`str`
    Its ``Position.buf``

fallback: :class:`int`
    The answer when nothing else gives one :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``4``

Returns
-------
:class:`int`
    The influences per vertex
    )doc");

    wwmiBlend.def_static("componentOfVertex", &AGRC::WWMIBlend::componentOfVertex, py::arg("indexPath"),
                         py::arg("drawRanges"), py::arg("vertices"), R"doc(
Which source component draws each vertex, off the mod's own draw ranges over its index buffer -- ``-1`` for
a vertex no range draws. Where two ranges reach one vertex, the later component in ``drawRanges`` wins

Parameters
----------
indexPath: :class:`str`
    The mod's index buffer

drawRanges: Dict[:class:`int`, List[Tuple[:class:`int`, :class:`int`]]]
    source component -> the (index count, first index) of each of its draws

vertices: :class:`int`
    How many vertices the mesh has

Returns
-------
List[:class:`int`]
    One component per vertex
    )doc");

    wwmiBlend.def_static("remapThroughVertexVG", [](const std::string& blendPath, const std::string& fixedPath,
                                                    const std::string& vertexVGPath, const std::string& positionPath,
                                                    const py::object& row) {
        AGRC::WWMIBlend::remapThroughVertexVG(blendPath, fixedPath, vertexVGPath, positionPath, vgRemapFromPy(row));
    }, py::arg("blendPath"), py::arg("fixedPath"), py::arg("vertexVGPath"), py::arg("positionPath"), py::arg("row"), R"doc(
Remaps the blend of a mod whose skeleton passes 256 bones: each weighted slot's id is read from its
``BlendRemapVertexVG.buf`` (the one-byte id in the blend is only an index into the mod's blend remap) and
sent through ``row``. The weights are kept

Parameters
----------
blendPath: :class:`str`
    The mod's ``Blend.buf``

fixedPath: :class:`str`
    Where the remapped blend is written

vertexVGPath: :class:`str`
    The mod's ``BlendRemapVertexVG.buf``

positionPath: :class:`str`
    Its ``Position.buf``

row: Union[:class:`VGRemap`, Dict[:class:`int`, :class:`int`]]
    The vertex group remap
    )doc");

    wwmiBlend.def_static("liftLegacy", [](const std::string& blendPath, const std::string& fixedPath,
                                          const std::string& positionPath, const AGRC::WWMIBlend::LegacyLift& legacy,
                                          const py::object& row) {
        AGRC::WWMIBlend::liftLegacy(blendPath, fixedPath, positionPath, legacy, vgRemapFromPy(row));
    }, py::arg("blendPath"), py::arg("fixedPath"), py::arg("positionPath"), py::arg("legacy"), py::arg("row"), R"doc(
Remaps the blend of a mod from before WWMI's merged skeleton: each weighted slot's id is local to the
component that draws the vertex (:meth:`componentOfVertex`), so it is lifted through that component's
``vg_map`` and only then sent through ``row``. A vertex no component with a ``vg_map`` draws is left alone

Parameters
----------
blendPath: :class:`str`
    The mod's ``Blend.buf``

fixedPath: :class:`str`
    Where the remapped blend is written

positionPath: :class:`str`
    Its ``Position.buf``

legacy: :class:`WWMIBlend.LegacyLift`
    The mod's index buffer, draw ranges and ``vg_map``\\s

row: Union[:class:`VGRemap`, Dict[:class:`int`, :class:`int`]]
    The vertex group remap
    )doc");

    wwmiBlend.def_static("writeRemap", [](const std::string& blendPath, const std::string& fixedPath,
                                          const std::string& srcVertexVGPath, const std::string& positionPath,
                                          const py::object& row, const AGRC::WWMIBlend::RemapFiles& out,
                                          const std::optional<AGRC::WWMIBlend::LegacyLift>& legacy) {
        AGRC::WWMIBlend::writeRemap(blendPath, fixedPath, srcVertexVGPath, positionPath, vgRemapFromPy(row), out, legacy);
    }, py::arg("blendPath"), py::arg("fixedPath"), py::arg("srcVertexVGPath"), py::arg("positionPath"), py::arg("row"),
       py::arg("out"), py::arg("legacy") = py::none(), R"doc(
Remaps a blend ONTO a character whose merged skeleton passes 256 bones, writing WWMI's blend remap for it

One remap serves the whole mesh, over the distinct targets ``row`` names, so a caller can declare the
remap's bone count from the row alone. The blend gets the LOCAL id each mapped bone has in that remap,
which is what WWMI's BlendRemapper writes there at run time, and the weights are kept. The true ids are
the mod's own ``BlendRemapVertexVG.buf`` when it has one, else the blend's, lifted as :meth:`liftLegacy`
lifts them when ``legacy`` is given

Parameters
----------
blendPath: :class:`str`
    The mod's ``Blend.buf``

fixedPath: :class:`str`
    Where the remapped blend is written

srcVertexVGPath: :class:`str`
    The mod's own ``BlendRemapVertexVG.buf``, or ``""`` for none

positionPath: :class:`str`
    Its ``Position.buf``

row: Union[:class:`VGRemap`, Dict[:class:`int`, :class:`int`]]
    The vertex group remap

out: :class:`WWMIBlend.RemapFiles`
    Where the remap's three buffers are written

legacy: Optional[:class:`WWMIBlend.LegacyLift`]
    The mod's legacy lift, for a mod from before WWMI's merged skeleton :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Raises
------
:class:`RuntimeError`
    Also when ``row`` maps to a bone a remap of :attr:`RemapSize` entries cannot name
    )doc");

    wwmiBlend.def_static("widen", &AGRC::WWMIBlend::widen, py::arg("path"), py::arg("vertices"), py::arg("fromInfluences"),
                         py::arg("toInfluences"), py::arg("idBytes"), py::arg("hasWeights"), R"doc(
Pads every vertex of a per-vertex buffer, in place, from ``fromInfluences`` slots to ``toInfluences``: ``fromInfluences`` ids of
``idBytes`` bytes, then -- for a blend line -- ``fromInfluences`` one-byte weights. The new slots are bone 0 at
weight 0, which no shader weighs. A file already ``toInfluences`` wide is left alone

Parameters
----------
path: :class:`str`
    The buffer

vertices: :class:`int`
    How many vertices it holds

fromInfluences: :class:`int`
    How many slots it has

toInfluences: :class:`int`
    How many it should have

idBytes: :class:`int`
    The bytes of one id: ``1`` in a blend, ``2`` in a ``BlendRemapVertexVG.buf``

hasWeights: :class:`bool`
    Whether each line carries weights after its ids
    )doc");

    wwmiBlend.def_static("narrow", &AGRC::WWMIBlend::narrow, py::arg("path"), py::arg("vertexVG"), py::arg("vertices"),
                         py::arg("fromInfluences"), py::arg("toInfluences"), R"doc(
Narrows every vertex of a blend, in place, from ``fromInfluences`` slots to ``toInfluences``

Slots of one vertex naming the same bone are merged first, their weights summed onto the first; then each
vertex keeps its ``toInfluences`` heaviest slots (a tie goes to the lower slot), their weights scaled so the vertex's
total is what it was, the rounding remainder on the heaviest. ``vertexVG``, when given, names the bones and
keeps the SAME slots. A blend already ``toInfluences`` wide is left alone

Parameters
----------
path: :class:`str`
    The blend

vertexVG: :class:`str`
    The ``BlendRemapVertexVG.buf`` beside it, or ``""`` for none

vertices: :class:`int`
    How many vertices it holds

fromInfluences: :class:`int`
    How many slots it has

toInfluences: :class:`int`
    How many it should keep
    )doc");
}


void initCppSkeletonNumberingProbe(pybind11::module_ &m) {
    py::class_<AGRC::SkeletonNumberingProbe> probe(m, "SkeletonNumberingProbe", R"doc(
Tells which of several numberings a mod's blend gives its bone ids in, from the mod's own geometry

A character whose skeleton was renumbered between game versions has mods whose blends name bones by an
older numbering, while the community hash tools rewrite a mod's hashes and keep its bone ids -- so the
hashes cannot say which numbering a blend is in. The geometry can: for every vertex whose heaviest bone
(over half its weight) is an id the numberings disagree on, each numbering is scored by the vertex's
distance to that bone's rest-pose centroid on the character's own model, and the numbering with the
smallest median distance wins

The buffers are a Wuthering Waves (WWMI) mod's: ``Position.buf`` of three 32-bit floats a vertex,
``Blend.buf`` of the bone ids then the weights of each vertex, 8 bits each, and optionally the 16-bit ids
of a mod past 256 bones (``BlendRemapVertexVG.buf``), read in place of the blend's own
    )doc");

    py::class_<AGRC::SkeletonNumberingProbe::Numbering>(probe, "Numbering", R"doc(
One way a character's bone ids have been numbered
    )doc")
        .def(py::init([](std::string version, std::unordered_map<long long, long long> toReference) {
                 return AGRC::SkeletonNumberingProbe::Numbering{std::move(version), std::move(toReference)};
             }), py::arg("version") = std::string(), py::arg("toReference") = std::unordered_map<long long, long long>{})
        .def_readwrite("version", &AGRC::SkeletonNumberingProbe::Numbering::version,
                        py::doc(":class:`str`: The version whose bone ids follow this numbering"))
        .def_readwrite("toReference", &AGRC::SkeletonNumberingProbe::Numbering::toReference, py::doc(R"doc(
Dict[:class:`int`, :class:`int`]: Id in this numbering -> the same bone's id in the numbering the centroids
are keyed by. An id not listed is the same in both
        )doc"));

    py::class_<AGRC::SkeletonNumberingProbe::Result>(probe, "Result", R"doc(
What :meth:`SkeletonNumberingProbe.probe` found
    )doc")
        .def_readonly("best", &AGRC::SkeletonNumberingProbe::Result::best,
                      py::doc(":class:`int`: The index of the numbering with the smallest median distance"))
        .def_readonly("medians", &AGRC::SkeletonNumberingProbe::Result::medians,
                      py::doc("List[:class:`float`]: Each numbering's median distance, in the order the numberings were given"))
        .def_readonly("vertices", &AGRC::SkeletonNumberingProbe::Result::vertices,
                      py::doc(":class:`int`: How many vertices sit on a bone the numberings disagree on"));

    probe.def_readonly_static("MinVertices", &AGRC::SkeletonNumberingProbe::MinVertices,
                              py::doc(":class:`int`: The fewest vertices on disputed bones :meth:`probe` gives an answer from"));
    probe.def_static("probe", &AGRC::SkeletonNumberingProbe::probe, py::arg("positionFile"), py::arg("blendFile"),
                     py::arg("vertexVGFile") = py::none(), py::arg("numberings") = std::vector<AGRC::SkeletonNumberingProbe::Numbering>{},
                     py::arg("centroids") = std::unordered_map<long long, std::array<double, 3>>{}, R"doc(
Which numbering the mod's blend is in

Parameters
----------
positionFile: :class:`str`
    The mod's ``Position.buf``

blendFile: :class:`str`
    The mod's ``Blend.buf``

vertexVGFile: Optional[:class:`str`]
    The mod's 16-bit bone ids, read in place of the blend's own :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

numberings: List[:class:`SkeletonNumberingProbe.Numbering`]
    The candidate numberings -- at least two

centroids: Dict[:class:`int`, Tuple[:class:`float`, :class:`float`, :class:`float`]]
    Each bone's rest-pose centroid on the character's own model, keyed by its reference id

Returns
-------
Optional[:class:`SkeletonNumberingProbe.Result`]
    The answer, or ``None`` when fewer than two numberings are given, a buffer cannot be read or its size
    does not fit the others, or fewer than :attr:`MinVertices` vertices sit on disputed bones
    )doc");
}


void initCppWWMIShapeKeys(pybind11::module_ &m) {
    py::class_<AGRC::WWMIShapeKeys> shapeKeys(m, "WWMIShapeKeys", R"doc(
The three shape-key buffers of a Wuthering Waves (WWMI) mod, as WWMI Tools writes them

* ``ShapeKeyOffset`` holds the FIRST entry of each key slot, one ``uint32`` a slot
* ``ShapeKeyVertexId`` holds one ``uint32`` vertex id per entry
* ``ShapeKeyVertexOffset`` holds one vertex offset of a fixed width per entry

A slot's entries run to the next slot's first, the last slot's to the end

The game drives a character's shape keys -- blinks, mouth shapes -- by SLOT, and two skins of one
character can number the same keys differently, so a mod remapped between them has its keys put in the
target's order by :meth:`reorder`

.. note::
    Every method is static; the class holds nothing
    )doc");

    py::class_<AGRC::WWMIShapeKeys::Files>(shapeKeys, "Files", R"doc(
The three buffers of one set of shape keys

Parameters
----------
offsets: :class:`str`
    ``ShapeKeyOffset``: each slot's first entry

vertexIds: :class:`str`
    ``ShapeKeyVertexId``: each entry's vertex

vertexOffsets: :class:`str`
    ``ShapeKeyVertexOffset``: each entry's offset
    )doc")
        .def(py::init([](std::string offsets, std::string vertexIds, std::string vertexOffsets) {
            return AGRC::WWMIShapeKeys::Files{std::move(offsets), std::move(vertexIds), std::move(vertexOffsets)};
        }), py::arg("offsets"), py::arg("vertexIds"), py::arg("vertexOffsets"))
        .def_readwrite("offsets", &AGRC::WWMIShapeKeys::Files::offsets, ":class:`str`: Each slot's first entry")
        .def_readwrite("vertexIds", &AGRC::WWMIShapeKeys::Files::vertexIds, ":class:`str`: Each entry's vertex")
        .def_readwrite("vertexOffsets", &AGRC::WWMIShapeKeys::Files::vertexOffsets, ":class:`str`: Each entry's offset");

    py::class_<AGRC::WWMIShapeKeys::Layout>(shapeKeys, "Layout", R"doc(
What a set of shape-key buffers holds
    )doc")
        .def(py::init<>())
        .def_readwrite("firsts", &AGRC::WWMIShapeKeys::Layout::firsts,
                       "List[:class:`int`]: Each slot's first entry. Empty when the buffers are not this layout")
        .def_readwrite("entries", &AGRC::WWMIShapeKeys::Layout::entries, ":class:`int`: How many entries there are")
        .def_readwrite("offsetWidth", &AGRC::WWMIShapeKeys::Layout::offsetWidth,
                       ":class:`int`: The bytes of one vertex offset. Zero when the buffers are not this layout");

    shapeKeys.def_static("layoutOf", &AGRC::WWMIShapeKeys::layoutOf, py::arg("files"), R"doc(
Reads the layout off the three buffers' bytes and sizes

Parameters
----------
files: :class:`WWMIShapeKeys.Files`
    The buffers

Returns
-------
:class:`WWMIShapeKeys.Layout`
    The layout, with no slots when the sizes do not divide as this layout's do

Raises
------
:class:`RuntimeError`
    If the offsets cannot be read
    )doc");

    shapeKeys.def_static("checkReorder", &AGRC::WWMIShapeKeys::checkReorder, py::arg("files"), py::arg("order"), R"doc(
Why the buffers cannot be put in ``order`` -- empty when they can

Parameters
----------
files: :class:`WWMIShapeKeys.Files`
    The buffers

order: List[:class:`int`]
    The order :meth:`reorder` would be asked for

Returns
-------
:class:`str`
    The reason, or ``""``
    )doc");

    py::class_<AGRC::WWMIShapeKeys::Bytes>(shapeKeys, "Bytes", R"doc(
The three buffers of one set of shape keys, as bytes -- see :meth:`WWMIShapeKeys.reordered`
    )doc")
        .def_property_readonly("offsets", [](const AGRC::WWMIShapeKeys::Bytes& self) {
            return py::bytes(reinterpret_cast<const char*>(self.offsets.data()), self.offsets.size());
        }, ":class:`bytes`: ``ShapeKeyOffset``")
        .def_property_readonly("vertexIds", [](const AGRC::WWMIShapeKeys::Bytes& self) {
            return py::bytes(reinterpret_cast<const char*>(self.vertexIds.data()), self.vertexIds.size());
        }, ":class:`bytes`: ``ShapeKeyVertexId``")
        .def_property_readonly("vertexOffsets", [](const AGRC::WWMIShapeKeys::Bytes& self) {
            return py::bytes(reinterpret_cast<const char*>(self.vertexOffsets.data()), self.vertexOffsets.size());
        }, ":class:`bytes`: ``ShapeKeyVertexOffset``");

    shapeKeys.def_static("reordered", &AGRC::WWMIShapeKeys::reordered, py::arg("files"), py::arg("order"), R"doc(
The buffers :meth:`reorder` writes, as bytes -- for a caller that hands them to the fix's resources rather
than writing them itself

Parameters
----------
files: :class:`WWMIShapeKeys.Files`
    The buffers to read

order: List[:class:`int`]
    The source key for each slot -- see :meth:`reorder`

Raises
------
:class:`RuntimeError`
    If a buffer cannot be read, or the reorder would change the number of entries

Returns
-------
:class:`WWMIShapeKeys.Bytes`
    The reordered buffers
    )doc");

    shapeKeys.def_static("reorder", &AGRC::WWMIShapeKeys::reorder, py::arg("files"), py::arg("order"), py::arg("out"), R"doc(
Writes the buffers again with their slots in another order

``order[j]`` is the key whose entries go in slot ``j``, ``-1`` for a slot that takes none. Slots past the
end of ``order`` keep their entries and position. A key of the first ``len(order)`` that no slot takes is
kept, its offsets zeroed, after the last ordered slot, so the number of entries does not change

Parameters
----------
files: :class:`WWMIShapeKeys.Files`
    The buffers to read

order: List[:class:`int`]
    The source key for each slot

out: :class:`WWMIShapeKeys.Files`
    Where the reordered buffers are written

Raises
------
:class:`RuntimeError`
    If a buffer cannot be read or written, or the reorder would change the number of entries
    )doc");
}


void initCppRabbitFX(pybind11::module_ &m) {
    py::class_<AGRC::RabbitFX> rabbitFX(m, "RabbitFX", R"doc(
What a fix needs to know about RabbitFX 8.3.2, the Wuthering Waves effects library a mod calls through
:attr:`IniKeywords.RabbitFXSetTexturesPath`

RabbitFX's ``SetTextures`` decides how to route the maps a draw has bound at ``ps-t0`` .. ``ps-t3`` by a
TAG its texture overrides give each texture (a ``filter_index`` of ``1718.3`` .. ``1718.6``), and writes
the result as four routing flags, ``x187`` .. ``w187``. A remapped draw binds the TARGET's textures there,
so the flags the source's draw would have had are worked out from the source's textures
(:meth:`textureTag`, :meth:`routingFlags`) and pinned after the call (:attr:`FlagKeys`)

.. note::
    Every method is static; the class holds nothing
    )doc");

    rabbitFX.attr("FlagKeys") = py::cast(AGRC::RabbitFX::FlagKeys);

    rabbitFX.def_static("textureTag", &AGRC::RabbitFX::textureTag, py::arg("path"), R"doc(
The tag RabbitFX's texture overrides give the texture at ``path``, from its ``.dds`` header: ``3`` for
``filter_index = 1718.3`` .. ``6`` for ``1718.6``, ``0`` for none

.. note::
    A square ``BC7`` of at most 512 pixels with ONE mip matches both ``1718.4`` and ``1718.6``; it is
    taken as ``6``

Parameters
----------
path: :class:`str`
    The ``.dds`` file

Returns
-------
:class:`int`
    The tag
    )doc");

    rabbitFX.def_static("routingFlags", &AGRC::RabbitFX::routingFlags, py::arg("tags"), R"doc(
The routing flags ``SetTextures`` writes for a draw whose ``ps-t0`` .. ``ps-t3`` carry ``tags``. A flag it
leaves unwritten keeps whatever the previous draw left, which no fix can know; it is ``0`` here

Parameters
----------
tags: List[:class:`int`]
    Each register's :meth:`textureTag`, four of them

Returns
-------
List[:class:`int`]
    The flags, in :attr:`FlagKeys`' order
    )doc");

    rabbitFX.def_static("names", &AGRC::RabbitFX::names, py::arg("text"), R"doc(
Whether ``text`` names RabbitFX, ignoring case

Parameters
----------
text: :class:`str`
    An ``.ini`` line, key or value

Returns
-------
:class:`bool`
    Whether it does
    )doc");

    rabbitFX.def_static("isOwnKVP", &AGRC::RabbitFX::isOwnKVP, py::arg("key"), py::arg("value"), R"doc(
Whether a `KVP`_ is one of RabbitFX's own: a resource under :attr:`IniKeywords.RabbitFXResourceFolder`, or
a ``run =`` of one of its command lists

Parameters
----------
key: :class:`str`
    The key

value: :class:`str`
    The value

Returns
-------
:class:`bool`
    Whether it is
    )doc");
}


void initCppDrawUVCoverage(pybind11::module_ &m) {
    py::class_<AGRC::DrawUVCoverage> coverage(m, "DrawUVCoverage", R"doc(
Which texels of a texture the vertices of one ``drawindexed`` range land on

Built once from a mesh's index list and the U and V of each vertex, then asked per range: of the range's
distinct vertices, how many sit, at their UV, on a texel a test accepts. That is how a fix tells which of a
mod's draws paint a given part of an atlas

A UV is wrapped into ``[0, 1)`` the way a sampler in wrap mode reads it, and a UV that is not a finite
number is read as ``0``

Parameters
----------
indices: List[:class:`int`]
    The mesh's index list -- see :meth:`IbFile.getIndices`

us: List[:class:`float`]
    Each vertex's U

vs: List[:class:`float`]
    Each vertex's V

vertices: :class:`int`
    How many vertices the mesh has. An index at or past it is skipped
    )doc");

    py::class_<AGRC::DrawUVCoverage::Count>(coverage, "Count", R"doc(
What :meth:`DrawUVCoverage.count` found for one range
    )doc")
        .def(py::init<>())
        .def_readwrite("vertices", &AGRC::DrawUVCoverage::Count::vertices, ":class:`int`: How many distinct vertices the range draws")
        .def_readwrite("matched", &AGRC::DrawUVCoverage::Count::matched, ":class:`int`: How many of them land on a texel the test accepts");

    coverage.def(py::init<std::vector<long long>, std::vector<double>, std::vector<double>, std::size_t>(),
                 py::arg("indices"), py::arg("us"), py::arg("vs"), py::arg("vertices"));

    coverage.def_static("texelOf", &AGRC::DrawUVCoverage::texelOf, py::arg("u"), py::arg("v"), py::arg("width"), py::arg("height"), R"doc(
The texel a UV lands on in a texture of ``width`` x ``height``, wrapped into the texture

Parameters
----------
u: :class:`float`
    The U

v: :class:`float`
    The V

width: :class:`int`
    The texture's width

height: :class:`int`
    The texture's height

Returns
-------
Tuple[:class:`int`, :class:`int`]
    (x, y)
    )doc");

    coverage.def("count", &AGRC::DrawUVCoverage::count, py::arg("firstIndex"), py::arg("indexCount"), py::arg("width"),
                 py::arg("height"), py::arg("test"), R"doc(
Counts one ``drawindexed`` range's distinct vertices, and those landing on texels ``test`` accepts

Parameters
----------
firstIndex: :class:`int`
    The range's first index

indexCount: :class:`int`
    How many indices it draws. The part past the end of the index list is not read

width: :class:`int`
    The width of the texture the texels are in

height: :class:`int`
    Its height

test: Callable[[:class:`int`, :class:`int`], :class:`bool`]
    Which texels count, asked with (x, y)

Returns
-------
:class:`DrawUVCoverage.Count`
    The counts
    )doc");
}
