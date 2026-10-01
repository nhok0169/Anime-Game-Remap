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

#include "PyVGSplitGroupResource.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "../../tools/PyRefFunction.h"
#include "../buffers/PyVGComponentSplit.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


AGRC::VGSplitGroupConfig::LineEdit lineEditFromPy(const py::object &edit) {
    if (edit.is_none()) {
        return nullptr;
    }

    py::object held = edit;
    return [held](const AGRC::ByteVec &line) {
        py::bytes in(reinterpret_cast<const char*>(line.data()), line.size());
        std::string out = py::cast<std::string>(py::bytes(held(in)));
        return AGRC::ByteVec(out.begin(), out.end());
    };
}


PyVGSplitGroupResource::PyVGSplitGroupResource(std::string name, py::dict resources, AGRC::VGSplitGroupConfig config,
                                               std::function<bool(AGRC::IniGroupedResource&)> fixFunc, bool isBuilt):
    PyIniGroupedResource(std::move(name), std::move(resources), std::move(fixFunc), isBuilt),
    config(std::move(config)), texcoordLineEditObj(py::none()), positionLineEditObj(py::none()), mirrorLineEditObj(py::none()) {}


bool PyVGSplitGroupResource::_fix() {
    return AGRC::fixVGSplitGroup(*this, config, logger.get());
}


void initCppVGSplitGroupResource(pybind11::module_ &m) {
    py::class_<AGRC::VGPushAway>(m, "VGPushAway", R"doc(
A push of cloth HORIZONTALLY away from a point: every vertex on :attr:`groups` moves by :attr:`distance` times its
weight share on them, away from :attr:`from_`'s (x, z)
    )doc")
        .def(py::init([](std::vector<long long> groups, std::array<float, 3> from, float distance, int side) {
            return AGRC::VGPushAway{std::move(groups), from, distance, side};
        }), py::arg("groups") = std::vector<long long>{}, py::arg("from_") = std::array<float, 3>{0.0f, 0.0f, 0.0f},
            py::arg("distance") = 0.0f, py::arg("side") = 0)
        .def_readwrite("groups", &AGRC::VGPushAway::groups, py::doc("List[:class:`int`]: The SOURCE vertex groups whose vertices are pushed"))
        .def_readwrite("from_", &AGRC::VGPushAway::from, py::doc("List[:class:`float`]: The point pushed away from; only its (x, z) counts"))
        .def_readwrite("distance", &AGRC::VGPushAway::distance, py::doc(":class:`float`: How far a vertex wholly on :attr:`groups` moves"))
        .def_readwrite("side", &AGRC::VGPushAway::side, py::doc(":class:`int`: Only vertices with x > 0 (``1``), x < 0 (``-1``), or both (``0``)"));

    using Triangles = AGRC::InnerLayerOutline::Triangles;
    using Vec3 = AGRC::InnerLayerOutline::Vec3;
    const auto pointers = [](const std::vector<Triangles>& lists) {
        std::vector<const Triangles*> result;
        for (const Triangles& list : lists) {
            result.push_back(&list);
        }
        return result;
    };

    py::class_<AGRC::InnerLayerOutline>(m, "InnerLayerOutline", R"doc(
Which vertices of a mesh's INNER layers draw no outline (vertex colour alpha 0). The outline pass redraws a mesh as a
shell pushed out along each vertex's outline normal; on hair of close two-sided sheets, the inner face's shell can come
out in front of the outer face as small dark shards when the target's outline sits further out. A target triangle is inner when at least two of its corners are :meth:`covered`, or with
:attr:`facingAxis` when its face points in towards the vertical axis through the targets' centre -- and the decision
takes all three corners, since a triangle with its corners at different widths stretches its shell into a wedge
    )doc")
        .def(py::init([](float reach, bool facingAxis, float facingCos) {
            AGRC::InnerLayerOutline result;
            result.reach = reach;
            result.facingAxis = facingAxis;
            result.facingCos = facingCos;
            return result;
        }), py::arg("reach") = 0.1f, py::arg("facingAxis") = true, py::arg("facingCos") = 0.2f)
        .def_readwrite("reach", &AGRC::InnerLayerOutline::reach,
                       py::doc(":class:`float`: How far along its normal a vertex looks for a covering layer, in model units"))
        .def_readwrite("facingAxis", &AGRC::InnerLayerOutline::facingAxis,
                       py::doc(":class:`bool`: Whether a face turned in towards the targets' vertical axis is inner too"))
        .def_readwrite("facingCos", &AGRC::InnerLayerOutline::facingCos,
                       py::doc(":class:`float`: How far in a face must point to count as facing the axis (cosine below ``-facingCos``)"))
        .def("covered", [pointers](const AGRC::InnerLayerOutline& self, const std::vector<Vec3>& positions, const std::vector<Vec3>& normals,
                                   const std::vector<Triangles>& occluders, const std::vector<Triangles>& targets) {
            return self.covered(positions, normals, pointers(occluders), pointers(targets));
        }, py::arg("positions"), py::arg("normals"), py::arg("occluders"), py::arg("targets"), R"doc(
Whether each target vertex's normal runs into an occluder triangle (not one of its own) within :attr:`reach`

Parameters
----------
positions: List[Tuple[:class:`float`, :class:`float`, :class:`float`]]
    Per vertex, its position

normals: List[Tuple[:class:`float`, :class:`float`, :class:`float`]]
    Per vertex, its normal

occluders: List[List[Tuple[:class:`int`, :class:`int`, :class:`int`]]]
    Every triangle list that can cover a layer

targets: List[List[Tuple[:class:`int`, :class:`int`, :class:`int`]]]
    The triangle lists whose corners are asked about

Returns
-------
List[:class:`bool`]
    Per vertex
        )doc")
        .def("find", [pointers](const AGRC::InnerLayerOutline& self, const std::vector<Vec3>& positions, const std::vector<Vec3>& normals,
                                const std::vector<Triangles>& occluders, const std::vector<Triangles>& targets) {
            return self.find(positions, normals, pointers(occluders), pointers(targets));
        }, py::arg("positions"), py::arg("normals"), py::arg("occluders"), py::arg("targets"), R"doc(
The vertices whose outline goes: every corner of every inner target triangle

Parameters
----------
positions: List[Tuple[:class:`float`, :class:`float`, :class:`float`]]
    Per vertex, its position

normals: List[Tuple[:class:`float`, :class:`float`, :class:`float`]]
    Per vertex, its normal

occluders: List[List[Tuple[:class:`int`, :class:`int`, :class:`int`]]]
    Every triangle list that can cover a layer -- the whole mesh

targets: List[List[Tuple[:class:`int`, :class:`int`, :class:`int`]]]
    The triangle lists that may lose their outline -- the hair

Returns
-------
List[:class:`bool`]
    Per vertex
        )doc")
        .def_static("readPositions", [](const py::bytes& buffer, std::size_t stride) {
            const std::string raw = buffer;
            std::vector<Vec3> positions, normals;
            AGRC::InnerLayerOutline::readPositions(AGRC::ByteVec(raw.begin(), raw.end()), stride, positions, normals);
            return py::make_tuple(positions, normals);
        }, py::arg("buffer"), py::arg("stride"), R"doc(
Reads positions and normals out of a GIMI ``Position.buf`` (``POSITION`` float3, ``NORMAL`` float3 at byte 12)

Parameters
----------
buffer: :class:`bytes`
    The buffer's bytes

stride: :class:`int`
    The bytes per line, at least 24

Raises
------
ValueError
    If the stride has no room for a normal, or the buffer is not a whole number of lines

Returns
-------
Tuple[List[Tuple[:class:`float`, :class:`float`, :class:`float`]], List[Tuple[:class:`float`, :class:`float`, :class:`float`]]]
    The positions and the normals
        )doc");

    py::class_<PyVGSplitGroupResource, PyIniGroupedResource, AGRC::RemapIniResourceMixin, py::smart_holder>(m, "VGSplitGroupResource", R"doc(
This class inherits from :class:`IniGroupedResource` and :class:`RemapIniResourceMixin`

A group of one mod's buffers -- its ``Blend.buf``, ``Position.buf``, ``Texcoord.buf`` and ``.ib``
files -- split for one component of a multi-component target, together

The blend decides which vertices a component keeps, the index buffers decide which triangles, and
every vertex buffer then has to follow the same vertex set, renumbered the same way (issue #190) --
so the members are fixed from one :class:`VGComponentSplit` rather than one at a time. Members are
told apart by their ``type``: ``blend`` (exactly one), ``position`` and ``texcoord`` (at most one
each) and ``buf`` (the index buffers, any number), each read from its ``srcPath`` and written to its
``fixedPath``. Built for :class:`ResGroupCollect` through an :class:`IniGroupedResBuilder`

Parameters
----------
name: :class:`str`
    The name of the group

resources: Optional[Dict[Any, Any]]
    The group's members. If ``None``, a fresh empty ``dict`` is used :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

component: :class:`str`
    The component this group's files are written for :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

specs: Optional[List[:class:`VGComponentSpec`]]
    Every component of the target -- the split is joint :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

ibPaths: Optional[List[:class:`str`]]
    The source ``.ib`` of every drawn object, in draw order, whether or not this group holds it: a
    cut component's vertex set is the union over every object's kept triangles. ``None``: the
    group's own ``buf`` members :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

texcoordLineEdit: Optional[Callable[[:class:`bytes`], :class:`bytes`]]
    Applied to every line of the ``Texcoord.buf`` before filtering :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

positionLineEdit: Optional[Callable[[:class:`bytes`], :class:`bytes`]]
    Applied to every line of the ``Position.buf`` before filtering :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

fixFunc: Optional[Callable[[:class:`IniGroupedResource`], :class:`bool`]]
    Custom function for fixing the group, overriding the split if given :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

isBuilt: :class:`bool`
    Whether the group is ready to be fixed :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc")
        .def(py::init([](std::string name, const py::object &resources, std::string component, const py::object &specs,
                         const py::object &ibPaths, const py::object &texcoordLineEdit, const py::object &positionLineEdit,
                         const PyOptionalCallable<bool(PyIniGroupedResource&)> &fixFunc, bool isBuilt) {
            py::dict resourcesDict = resources.is_none() ? py::dict() : resources.cast<py::dict>();

            AGRC::VGSplitGroupConfig config;
            config.component = std::move(component);
            if (!specs.is_none()) {
                config.specs = specs.cast<std::vector<AGRC::VGComponentSpec>>();
            }
            if (!ibPaths.is_none()) {
                config.ibPaths = ibPaths.cast<std::vector<std::string>>();
            }
            config.texcoordLineEdit = lineEditFromPy(texcoordLineEdit);
            config.positionLineEdit = lineEditFromPy(positionLineEdit);

            auto result = std::make_unique<PyVGSplitGroupResource>(std::move(name), std::move(resourcesDict), std::move(config),
                                                                    toPyRefFunction<bool(AGRC::IniGroupedResource&)>(fixFunc), isBuilt);
            result->texcoordLineEditObj = texcoordLineEdit;
            result->positionLineEditObj = positionLineEdit;
            return result;
        }), py::arg("name"), py::arg("resources") = py::none(), py::arg("component") = "", py::arg("specs") = py::none(),
            py::arg("ibPaths") = py::none(), py::arg("texcoordLineEdit") = py::none(), py::arg("positionLineEdit") = py::none(),
            py::arg("fixFunc") = py::none(), py::arg("isBuilt") = true)
        .def_property("component", [](const PyVGSplitGroupResource &self) { return self.config.component; },
                      [](PyVGSplitGroupResource &self, std::string component) { self.config.component = std::move(component); },
                      py::doc(":class:`str`: The component this group's files are written for"))
        .def_property("specs", [](const PyVGSplitGroupResource &self) { return self.config.specs; },
                      [](PyVGSplitGroupResource &self, std::vector<AGRC::VGComponentSpec> specs) { self.config.specs = std::move(specs); },
                      py::doc("List[:class:`VGComponentSpec`]: Every component of the target"))
        .def_property("ibPaths", [](const PyVGSplitGroupResource &self) { return self.config.ibPaths; },
                      [](PyVGSplitGroupResource &self, std::vector<std::string> ibPaths) { self.config.ibPaths = std::move(ibPaths); },
                      py::doc("List[:class:`str`]: The source ``.ib`` of every drawn object, in draw order"))
        .def_property("texcoordLineEdit", [](const PyVGSplitGroupResource &self) { return self.texcoordLineEditObj; },
                      [](PyVGSplitGroupResource &self, const py::object &edit) {
                          self.texcoordLineEditObj = edit;
                          self.config.texcoordLineEdit = lineEditFromPy(edit);
                      }, py::doc("Optional[Callable[[:class:`bytes`], :class:`bytes`]]: Applied to every line of the ``Texcoord.buf``"))
        .def_property("positionLineEdit", [](const PyVGSplitGroupResource &self) { return self.positionLineEditObj; },
                      [](PyVGSplitGroupResource &self, const py::object &edit) {
                          self.positionLineEditObj = edit;
                          self.config.positionLineEdit = lineEditFromPy(edit);
                      }, py::doc("Optional[Callable[[:class:`bytes`], :class:`bytes`]]: Applied to every line of the ``Position.buf``"))
        .def_property("pushAway", [](const PyVGSplitGroupResource &self) { return self.config.pushAway; },
                      [](PyVGSplitGroupResource &self, std::vector<AGRC::VGPushAway> pushes) { self.config.pushAway = std::move(pushes); },
                      py::doc("List[:class:`VGPushAway`]: Pushes applied to the written ``Position.buf`` -- see :class:`VGPushAway`"))
        .def_property("innerOutline", [](const PyVGSplitGroupResource &self) { return self.config.innerOutline; },
                      [](PyVGSplitGroupResource &self, std::optional<AGRC::InnerLayerOutline> rule) { self.config.innerOutline = std::move(rule); },
                      py::doc("Optional[:class:`InnerLayerOutline`]: When set, the written ``Texcoord.buf`` draws no outline on the inner layers "
                              "of :attr:`innerOutlineIbs` -- decided on the SOURCE mesh, every index buffer covering. ``None`` by default"))
        .def_property("innerOutlineIbs", [](const PyVGSplitGroupResource &self) { return self.config.innerOutlineIbs; },
                      [](PyVGSplitGroupResource &self, std::vector<std::size_t> ibs) { self.config.innerOutlineIbs = std::move(ibs); },
                      py::doc("List[:class:`int`]: Which of :attr:`ibPaths`, by position, :attr:`innerOutline` asks about; empty for all"))
        .def_property("mirrorLineEdit", [](const PyVGSplitGroupResource &self) { return self.mirrorLineEditObj; },
                      [](PyVGSplitGroupResource &self, const py::object &edit) {
                          self.mirrorLineEditObj = edit;
                          self.config.mirrorLineEdit = lineEditFromPy(edit);
                      }, py::doc("Optional[Callable[[:class:`bytes`], :class:`bytes`]]: Applied, after filtering, to the ``Position.buf`` lines of the vertices the split mirrored -- see :meth:`VGComponentSplit.mirrorPositionLine`"));
}
