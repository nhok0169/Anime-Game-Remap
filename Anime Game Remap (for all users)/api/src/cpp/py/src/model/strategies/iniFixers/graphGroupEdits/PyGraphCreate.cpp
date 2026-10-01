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

#include "PyGraphCreate.h"

#include <memory>
#include <utility>

#include "PyIniGraphGroups.h"
#include "../../../PyIniSectionGraph.h"


PyGraphCreate::PyGraphCreate(py::object graphIdObj, py::object graphObj, bool minimal, bool newPartIds):
    Core(Core::GraphId(), nullptr, minimal, newPartIds),
    graphIdObj(std::move(graphIdObj)), graphObj(std::move(graphObj)) {}


void PyGraphCreate::refresh() {
    graphId = parseGraphId(graphIdObj);

    // Re-derived per edit rather than cached in the constructor, so reassigning `.graph` from Python
    // takes effect -- the convention every edit in this family follows. The Python object is what
    // keeps the graph alive across the call; the core member is only a borrowed view of it.
    graph = graphObj.is_none() ? nullptr : graphObj.cast<PyIniSectionGraph*>();
}


void initCppGraphCreate(pybind11::module_ &m) {
    py::class_<PyGraphCreate, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphCreate", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Adds a graph to a group of graphs, under a graph id of the caller's choosing --- the counterpart of
:class:`GraphRemove`, which takes one out

Where :class:`GraphGroupRemap` produces its graphs by copying ones the group already holds, this one
takes a graph the CALLER built and puts it in. That is what a fixer needs when the graph it wants to
render is not a copy of anything the parser handed it

.. note::
    **The graph is deep-copied in.** A group owns its graphs' lifetimes, so it can only be handed a
    graph it already owns --- this edit therefore adds a COPY of :attr:`graph`, which leaves the
    graph you passed yours and free to reuse for several ids

.. note::
    A :attr:`graphId` whose ``iniIndex`` is past the end of the groups is skipped silently, as
    :class:`GraphRemove` skips an out-of-range id --- **except** for an index of exactly
    ``len(graphGroups)``, which appends a fresh group and adds the graph to that. Appending the ONE
    group the index asks for is the useful case (a fix that renders an extra .ini file); an index
    far past the end is a mistake rather than a request for thousands of empty groups

.. note::
    An existing graph already stored under :attr:`graphId` is REPLACED

Parameters
----------
graphId: Tuple[:class:`int`, :class:`str`, :class:`str`]
    Where the graph goes. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`

    #. The index for the .ini file
    #. The name of the component
    #. The name of the mod object

graph: Optional[:class:`IniSectionGraph`]
    The graph to add. ``None`` makes the edit a no-op

minimal: :class:`bool`
    Whether the copy taken of the graph is a minimal one :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

newPartIds: :class:`bool`
    Whether the parts of the copy get fresh ids :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc");

    cls.def(py::init([](py::object graphId, py::object graph, bool minimal, bool newPartIds) {
        return std::make_unique<PyGraphCreate>(std::move(graphId), std::move(graph), minimal, newPartIds);
    }), py::arg("graphId"), py::arg("graph") = py::none(), py::arg("minimal") = true, py::arg("newPartIds") = true);

    cls.def_property("graphId", [](const PyGraphCreate &self) {
        return self.graphIdObj;
    }, [](PyGraphCreate &self, py::object graphId) {
        self.graphIdObj = std::move(graphId);
    }, py::doc(R"doc(
Tuple[:class:`int`, :class:`str`, :class:`str`]: Where the graph goes --- the .ini index, the
component name and the mod object name
    )doc"));

    cls.def_property("graph", [](const PyGraphCreate &self) {
        return self.graphObj;
    }, [](PyGraphCreate &self, py::object graph) {
        self.graphObj = std::move(graph);
    }, py::doc(R"doc(
Optional[:class:`IniSectionGraph`]: The graph to add, deep-copied in when the edit runs
    )doc"));

    cls.def_readwrite("minimal", &PyGraphCreate::minimal, py::doc(R"doc(
:class:`bool`: Whether the copy taken of :attr:`graph` is a minimal one
    )doc"));

    cls.def_readwrite("newPartIds", &PyGraphCreate::newPartIds, py::doc(R"doc(
:class:`bool`: Whether the parts of the copy taken of :attr:`graph` get fresh ids
    )doc"));

    cls.def("edit", [](PyGraphCreate &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        (void)modType;
        self.refresh();

        PyIniGraphGroups groups(graphGroups);

        // nullptr for the same reason GraphRemove's binding passes it -- this edit never reads it.
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Adds a copy of :attr:`graph` to 'graphGroups' under :attr:`graphId`

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix. Unused by this edit

modName: :class:`str`
    The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after the graph was added
    )doc"));
}
