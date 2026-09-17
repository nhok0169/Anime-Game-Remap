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

#include "PyVGMergeGroupResource.h"

#include <memory>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "../../tools/PyRefFunction.h"
#include "../buffers/PyVGComponentMerge.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyVGMergeGroupResource::PyVGMergeGroupResource(std::string name, py::dict resources, AGRC::VGMergeGroupConfig config,
                                               std::function<bool(AGRC::IniGroupedResource&)> fixFunc, bool isBuilt):
    PyIniGroupedResource(std::move(name), std::move(resources), std::move(fixFunc), isBuilt), config(std::move(config)) {}


bool PyVGMergeGroupResource::_fix() {
    return AGRC::fixVGMergeGroup(*this, config, logger.get());
}


void initCppVGMergeGroupResource(pybind11::module_ &m) {
    py::class_<AGRC::VGMergeComponentFiles>(m, "VGMergeComponentFiles", R"doc(
One source component's files, as a :class:`VGMergeGroupResource` finds them on disk

Parameters
----------
spec: :class:`VGMergeComponentSpec`
    The component and its own vertex group row

blendPath: :class:`str`
    This component's ``Blend.buf``

positionPath: :class:`str`
    This component's ``Position.buf``

texcoordPath: :class:`str`
    This component's ``Texcoord.buf``
    )doc")
        .def(py::init([](AGRC::VGMergeComponentSpec spec, std::string blendPath, std::string positionPath, std::string texcoordPath) {
            AGRC::VGMergeComponentFiles files;
            files.spec = std::move(spec);
            files.blendPath = std::move(blendPath);
            files.positionPath = std::move(positionPath);
            files.texcoordPath = std::move(texcoordPath);
            return files;
        }), py::arg("spec"), py::arg("blendPath") = "", py::arg("positionPath") = "", py::arg("texcoordPath") = "")
        .def_readwrite("spec", &AGRC::VGMergeComponentFiles::spec, py::doc(":class:`VGMergeComponentSpec`: The component and its row"))
        .def_readwrite("blendPath", &AGRC::VGMergeComponentFiles::blendPath, py::doc(":class:`str`: This component's ``Blend.buf``"))
        .def_readwrite("positionPath", &AGRC::VGMergeComponentFiles::positionPath, py::doc(":class:`str`: This component's ``Position.buf``"))
        .def_readwrite("texcoordPath", &AGRC::VGMergeComponentFiles::texcoordPath, py::doc(":class:`str`: This component's ``Texcoord.buf``"));

    py::class_<AGRC::VGMergeObject>(m, "VGMergeObject", R"doc(
One object of the TARGET, and the source objects drawn through it

Parameters
----------
srcPath: :class:`str`
    The source ``.ib`` this object's group member was collected from -- the representative, which is
    ``members``' first entry. The member resource is matched by this path

members: Optional[List[Tuple[:class:`str`, :class:`str`]]]
    The source objects this target object draws, as ``(component, .ib path)``, in draw order. More
    than one means a MERGE into a single draw -- YelanTranquil's ``Bang`` and ``Eye`` both land on
    Yelan's head :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")
        .def(py::init([](std::string srcPath, const py::object &members) {
            AGRC::VGMergeObject object;
            object.srcPath = std::move(srcPath);
            if (!members.is_none()) {
                object.members = members.cast<std::vector<std::pair<std::string, std::string>>>();
            }
            return object;
        }), py::arg("srcPath") = "", py::arg("members") = py::none())
        .def_readwrite("srcPath", &AGRC::VGMergeObject::srcPath, py::doc(":class:`str`: The source ``.ib`` the member was collected from"))
        .def_readwrite("members", &AGRC::VGMergeObject::members,
                       py::doc("List[Tuple[:class:`str`, :class:`str`]]: The source objects drawn through this one, as ``(component, .ib path)``"));

    py::class_<PyVGMergeGroupResource, PyIniGroupedResource, AGRC::RemapIniResourceMixin, py::smart_holder>(m, "VGMergeGroupResource", R"doc(
This class inherits from :class:`IniGroupedResource` and :class:`RemapIniResourceMixin`

A group of one mod's buffers merged from a source of SEVERAL components onto a single-component
target, together -- the inverse of :class:`VGSplitGroupResource`

A single-component target draws through one set of buffer hashes, so the components' buffers have to
become one set. Every component's index buffer then shifts by the vertices of the components before
it, which is why the members cannot be fixed one at a time. Members are told apart by their ``type``:
``blend``, ``position`` and ``texcoord`` (at most one each) and ``buf`` (the index buffers, matched
to a :class:`VGMergeObject`'s ``srcPath``)

Parameters
----------
name: :class:`str`
    The name of the group

resources: Optional[Dict[Any, Any]]
    The group's members. If ``None``, a fresh empty ``dict`` is used :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

components: Optional[List[:class:`VGMergeComponentFiles`]]
    Every component of the source, in merge order -- the first takes offset 0 :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

objects: Optional[List[:class:`VGMergeObject`]]
    The target's drawn objects :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

fixFunc: Optional[Callable[[:class:`IniGroupedResource`], :class:`bool`]]
    Custom function for fixing the group, overriding the merge if given :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

isBuilt: :class:`bool`
    Whether the group is ready to be fixed :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc")
        .def(py::init([](std::string name, const py::object &resources, const py::object &components, const py::object &objects,
                         const PyOptionalCallable<bool(PyIniGroupedResource&)> &fixFunc, bool isBuilt) {
            py::dict resourcesDict = resources.is_none() ? py::dict() : resources.cast<py::dict>();

            AGRC::VGMergeGroupConfig config;
            if (!components.is_none()) {
                config.components = components.cast<std::vector<AGRC::VGMergeComponentFiles>>();
            }
            if (!objects.is_none()) {
                config.objects = objects.cast<std::vector<AGRC::VGMergeObject>>();
            }

            return std::make_unique<PyVGMergeGroupResource>(std::move(name), std::move(resourcesDict), std::move(config),
                                                             toPyRefFunction<bool(AGRC::IniGroupedResource&)>(fixFunc), isBuilt);
        }), py::arg("name"), py::arg("resources") = py::none(), py::arg("components") = py::none(), py::arg("objects") = py::none(),
            py::arg("fixFunc") = py::none(), py::arg("isBuilt") = true)
        .def_property("components", [](const PyVGMergeGroupResource &self) { return self.config.components; },
                      [](PyVGMergeGroupResource &self, std::vector<AGRC::VGMergeComponentFiles> components) {
                          self.config.components = std::move(components);
                      }, py::doc("List[:class:`VGMergeComponentFiles`]: Every component of the source, in merge order"))
        .def_property("objects", [](const PyVGMergeGroupResource &self) { return self.config.objects; },
                      [](PyVGMergeGroupResource &self, std::vector<AGRC::VGMergeObject> objects) {
                          self.config.objects = std::move(objects);
                      }, py::doc("List[:class:`VGMergeObject`]: The target's drawn objects"));
}
