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

#include "PyVGComponentMerge.h"

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


AGRC::VGMergeComponentSpec vgMergeComponentSpecFromPy(const std::string &name, const py::object &remap) {
    AGRC::VGMergeComponentSpec spec;
    spec.name = name;
    if (py::isinstance<AGRC::VGRemap>(remap)) {
        spec.remap = remap.cast<AGRC::VGRemap>();
    } else if (!remap.is_none()) {
        spec.remap = AGRC::VGRemap(remap.cast<std::unordered_map<long long, long long>>());
    }
    return spec;
}


void initCppVGComponentMerge(pybind11::module_ &m) {
    py::class_<AGRC::VGMergeComponentSpec>(m, "VGMergeComponentSpec", R"doc(
One SOURCE component of a skin made of several -- YelanTranquil's ``Body``, ``Bang`` or ``Eye`` -- and
how its own vertex groups reach the target's bones

Parameters
----------
name: :class:`str`
    The component's name

remap: Union[:class:`VGRemap`, Dict[:class:`int`, :class:`int`]]
    This component's vertex group (source index) to the target's bone. Each component has a row of
    its OWN, which is the whole reason the components cannot be remapped together
    )doc")
        .def(py::init([](const std::string &name, const py::object &remap) {
            return vgMergeComponentSpecFromPy(name, remap);
        }), py::arg("name"), py::arg("remap") = py::none())
        .def_readwrite("name", &AGRC::VGMergeComponentSpec::name, py::doc(":class:`str`: The component's name"))
        .def_readwrite("remap", &AGRC::VGMergeComponentSpec::remap,
                       py::doc(":class:`VGRemap`: This component's vertex group to the target's bone"));

    py::class_<AGRC::VGComponentMergeStats>(m, "VGComponentMergeStats", R"doc(
Counts worth reporting about a merge
    )doc")
        .def_readonly("vertexCount", &AGRC::VGComponentMergeStats::vertexCount, py::doc(":class:`int`: The merged vertex count"))
        .def_readonly("order", &AGRC::VGComponentMergeStats::order, py::doc("List[:class:`str`]: The component names, in merge order"))
        .def_readonly("offsets", &AGRC::VGComponentMergeStats::offsets, py::doc("List[:class:`int`]: Per component, its first vertex in the merged buffers"))
        .def_readonly("vertices", &AGRC::VGComponentMergeStats::vertices, py::doc("List[:class:`int`]: Per component, how many vertices it brought"))
        .def_readonly("texcoordStride", &AGRC::VGComponentMergeStats::texcoordStride, py::doc(":class:`int`: The merged ``Texcoord.buf``'s stride -- the widest component's"))
        .def_readonly("paddedComponents", &AGRC::VGComponentMergeStats::paddedComponents, py::doc(":class:`int`: How many components' texcoord lines had to be padded"))
        .def_readonly("unmapped", &AGRC::VGComponentMergeStats::unmapped,
                      py::doc("List[Tuple[:class:`str`, :class:`int`]]: Every ``(component, vertex group)`` that carries weight and has no remap -- each becomes a NEGATIVE bone index and kinks the model"));

    py::class_<AGRC::VGComponentMerge::Component>(m, "VGMergeComponent", R"doc(
One source component's decoded blend and its raw vertex buffers

Parameters
----------
spec: :class:`VGMergeComponentSpec`
    The component and its own vertex group row

weights: List[List[:class:`float`]]
    Per vertex, its 4 blend weights

indices: List[List[:class:`int`]]
    Per vertex, its 4 vertex group indices, in THIS component's numbering

position: :class:`bytes`
    The whole ``Position.buf``, copied through unchanged

texcoord: :class:`bytes`
    The whole ``Texcoord.buf``, padded up to the widest component's stride
    )doc")
        .def(py::init([](AGRC::VGMergeComponentSpec spec, AGRC::VGComponentMerge::Weights weights,
                         AGRC::VGComponentMerge::Indices indices, const py::bytes &position, const py::bytes &texcoord) {
            AGRC::VGComponentMerge::Component component;
            component.spec = std::move(spec);
            component.weights = std::move(weights);
            component.indices = std::move(indices);
            component.position = fromBytes(position);
            component.texcoord = fromBytes(texcoord);
            return component;
        }), py::arg("spec"), py::arg("weights"), py::arg("indices"), py::arg("position"), py::arg("texcoord"))
        .def_readwrite("spec", &AGRC::VGComponentMerge::Component::spec, py::doc(":class:`VGMergeComponentSpec`: The component and its row"))
        .def_readwrite("weights", &AGRC::VGComponentMerge::Component::weights, py::doc("List[List[:class:`float`]]: Per vertex, its 4 blend weights"))
        .def_readwrite("indices", &AGRC::VGComponentMerge::Component::indices, py::doc("List[List[:class:`int`]]: Per vertex, its 4 vertex group indices"))
        .def_property("position", [](const AGRC::VGComponentMerge::Component &self) { return toBytes(self.position); },
                      [](AGRC::VGComponentMerge::Component &self, const py::bytes &bytes) { self.position = fromBytes(bytes); },
                      py::doc(":class:`bytes`: The whole ``Position.buf``"))
        .def_property("texcoord", [](const AGRC::VGComponentMerge::Component &self) { return toBytes(self.texcoord); },
                      [](AGRC::VGComponentMerge::Component &self, const py::bytes &bytes) { self.texcoord = fromBytes(bytes); },
                      py::doc(":class:`bytes`: The whole ``Texcoord.buf``"));

    py::class_<AGRC::VGComponentMerge>(m, "VGComponentMerge", R"doc(
Merges the geometry of a mod built for a skin of SEVERAL components onto a target of one -- the
inverse of :class:`VGComponentSplit`

A single-component target draws through ONE set of buffer hashes, so the components' buffers have to
become one set: several ``.ini`` files each binding that hash to different bytes does not work,
because a hash binds once and the other components' index buffers then address the winner's vertices

The components are laid end to end in the order given, so the first keeps its index buffers
unchanged. Each component's blend is remapped through its OWN row before the concatenation, and two
source objects landing on the same target object have their index buffers concatenated into one draw

Parameters
----------
components: List[:class:`VGMergeComponent`]
    The source's components; the first takes offset 0
    )doc")
        .def(py::init<std::vector<AGRC::VGComponentMerge::Component>>(), py::arg("components"))
        .def_property_readonly("vertexCount", &AGRC::VGComponentMerge::vertexCount, py::doc(":class:`int`: The merged vertex count"))
        .def_property_readonly("stats", &AGRC::VGComponentMerge::stats, py::doc(":class:`VGComponentMergeStats`: Counts worth reporting"))
        .def_property_readonly("blend", [](const AGRC::VGComponentMerge &self) { return toBytes(self.blend()); },
                               py::doc(":class:`bytes`: The merged ``Blend.buf``"))
        .def_property_readonly("position", [](const AGRC::VGComponentMerge &self) { return toBytes(self.position()); },
                               py::doc(":class:`bytes`: The merged ``Position.buf``"))
        .def_property_readonly("texcoord", [](const AGRC::VGComponentMerge &self) { return toBytes(self.texcoord()); },
                               py::doc(":class:`bytes`: The merged ``Texcoord.buf``"))
        .def("offsetOf", &AGRC::VGComponentMerge::offsetOf, py::arg("component"),
             py::doc("A component's first vertex in the merged buffers"))
        .def("mergeIbs", &AGRC::VGComponentMerge::mergeIbs, py::arg("members"), py::doc(R"doc(
One target object's index buffer: each member's triangles offset by its component's first vertex,
concatenated in the order given

Parameters
----------
members: List[Tuple[:class:`str`, List[List[:class:`int`]]]]
    The source objects drawn through this target object, as ``(component, triangles)``

Returns
-------
List[List[:class:`int`]]
    The merged triangles
        )doc"))
        .def_static("remapIndices", [](const AGRC::VGComponentMerge::Indices &indices, const AGRC::VGComponentMerge::Weights &weights,
                                       const AGRC::VGRemap &remap) {
            std::vector<long long> unmapped;
            AGRC::VGComponentMerge::Indices result = AGRC::VGComponentMerge::remapIndices(indices, weights, remap, unmapped);
            return std::make_pair(std::move(result), std::move(unmapped));
        }, py::arg("indices"), py::arg("weights"), py::arg("remap"), py::doc(R"doc(
One component's bone indices through its remap

A slot is remapped only where it carries WEIGHT -- a zero-weight slot keeps its literal index, so
``[0, 0, 0, 0]`` with weights ``[1, 0, 0, 0]`` comes out ``[64, 0, 0, 0]`` and not ``[64, 64, 64, 64]``

Returns
-------
Tuple[List[List[:class:`int`]], List[:class:`int`]]
    The remapped indices, and every weighted group that had no entry
        )doc"))
        .def_static("padLines", [](const py::bytes &src, std::size_t lines, std::size_t to) {
            return toBytes(AGRC::VGComponentMerge::padLines(fromBytes(src), lines, to));
        }, py::arg("src"), py::arg("lines"), py::arg("to"),
           py::doc("A fixed-stride buffer widened, every line zero-padded at the END -- where a missing ``TEXCOORD1`` sits"));
}
