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
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

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
    config(std::move(config)), texcoordLineEditObj(py::none()), positionLineEditObj(py::none()) {}


bool PyVGSplitGroupResource::_fix() {
    return AGRC::fixVGSplitGroup(*this, config, logger.get());
}


void initCppVGSplitGroupResource(pybind11::module_ &m) {
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
                         std::function<bool(AGRC::IniGroupedResource&)> fixFunc, bool isBuilt) {
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
                                                                    std::move(fixFunc), isBuilt);
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
                      }, py::doc("Optional[Callable[[:class:`bytes`], :class:`bytes`]]: Applied to every line of the ``Position.buf``"));
}
