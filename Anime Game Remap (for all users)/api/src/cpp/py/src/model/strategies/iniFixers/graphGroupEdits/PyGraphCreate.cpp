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
    graphIdObj(std::move(graphIdObj)), graphObj(std::move(graphObj)) {
    refresh();
}


void PyGraphCreate::refresh() {
    // Re-derived when built and whenever a property is set, so the C++ members are what a C++ fixer's list runs. The
    // Python object keeps the graph alive; the core member is only a borrowed view of it.
    graphId = parseGraphId(graphIdObj);

    // A callable is asked for the graph when the edit runs (GraphCreate::graphOf), and what it returns is kept alive
    graph = nullptr;
    graphOf = nullptr;
    if (!graphObj.is_none() && !py::isinstance<PyIniSectionGraph>(graphObj) && PyCallable_Check(graphObj.ptr())) {
        graphOf = [this]() -> Graph* {
            py::gil_scoped_acquire gil;
            graphOfResult = graphObj();
            return graphOfResult.is_none() ? nullptr : graphOfResult.cast<PyIniSectionGraph*>();
        };
    } else {
        graphOfResult = graphObj;
        graph = graphOfResult.is_none() ? nullptr : graphOfResult.cast<PyIniSectionGraph*>();
    }

    // unset (a null handle until the property is assigned) or None: no graph is built from sections
    if (!sectionsOfObj || sectionsOfObj.is_none()) {
        sectionsOf = nullptr;
        return;
    }
    sectionsOf = [this](std::size_t group) {
        py::gil_scoped_acquire gil;
        std::vector<Section*> sections;
        py::object result = sectionsOfObj(group);
        if (result.is_none()) {
            return sections;
        }
        sectionsOfResults.append(result);
        for (auto item : result) {
            sections.push_back(item.cast<Section*>());
        }
        return sections;
    };
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

graph: Optional[Union[:class:`IniSectionGraph`, Callable[[], Optional[:class:`IniSectionGraph`]]]]
    The graph to add, or a callable asked for it when the edit runs -- for a graph that only exists by then, such
    as a parser's global graph. ``None`` makes the edit a no-op

minimal: :class:`bool`
    Whether the copy taken of the graph is a minimal one :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

newPartIds: :class:`bool`
    Whether the parts of the copy get fresh ids :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc");

    // every setting of the edit can be given here, as the property of the same name
    cls.def(py::init([](py::object graphId, py::object graph, bool minimal, bool newPartIds, py::object sectionsOf,
                        AGRC::Z3Context* z3, py::object onlyWith, py::object onlyWithout) {
        auto edit = std::make_unique<PyGraphCreate>(std::move(graphId), std::move(graph), minimal, newPartIds);
        edit->z3 = z3;
        edit->onlyWith = PyIniGraphGroups::modObjsFromPy(onlyWith);
        edit->onlyWithout = PyIniGraphGroups::modObjsFromPy(onlyWithout);
        if (!sectionsOf.is_none()) {
            edit->sectionsOfObj = std::move(sectionsOf);
            edit->refresh();
        }
        return edit;
    }), py::arg("graphId"), py::arg("graph") = py::none(), py::arg("minimal") = true, py::arg("newPartIds") = true,
        py::arg("sectionsOf") = py::none(), py::arg("z3") = nullptr, py::arg("onlyWith") = py::none(),
        py::arg("onlyWithout") = py::none(), py::keep_alive<1, 7>());

    cls.def_property("graphId", [](const PyGraphCreate &self) {
        return self.graphIdObj;
    }, [](PyGraphCreate &self, py::object graphId) {
        self.graphIdObj = std::move(graphId);
        self.refresh();
    }, py::doc(R"doc(
Tuple[:class:`int`, :class:`str`, :class:`str`]: Where the graph goes --- the .ini index, the
component name and the mod object name
    )doc"));

    cls.def_property("graph", [](const PyGraphCreate &self) {
        return self.graphObj;
    }, [](PyGraphCreate &self, py::object graph) {
        self.graphObj = std::move(graph);
        self.refresh();
    }, py::doc(R"doc(
Optional[Union[:class:`IniSectionGraph`, Callable[[], Optional[:class:`IniSectionGraph`]]]]: The graph to add, or a callable asked for it when the edit runs -- deep-copied in when the edit runs
    )doc"));

    cls.def_property("sectionsOf", [](const PyGraphCreate &self) {
        return self.sectionsOfObj;
    }, [](PyGraphCreate &self, py::object sectionsOf) {
        self.sectionsOfObj = std::move(sectionsOf);
        self.refresh();
    }, py::doc(R"doc(
Optional[Callable[[:class:`int`], Optional[List[:class:`IfTemplate`]]]]: The `sections`_ of each group's graph, given the group's index :raw-html:`<br />` :raw-html:`<br />`

Set, the edit builds a graph from them for EVERY group present when it runs and adds it under :attr:`graphId`'s component
and object (its .ini index is unused), instead of adding :attr:`graph`. A section of a name already in the graph is left
out, so the first one wins; a group handed no sections gets no graph. When there are no groups at all, group 0's
sections go into a group of their own. This is how the sections a :class:`GraphEditVariables` variable holds are written
out as a graph in each group -- a fixer's own sections into its file and every copy. **Default**: ``None``

:example:

.. code-block:: python

    write = FRB.GraphCreate((0, "", "fixSections"))
    write.sectionsOf = lambda group: variables.sections("fix").list()
    )doc"));

    cls.def_property("onlyWithout", [](const PyGraphCreate &self) -> py::object {
        return PyIniGraphGroups::modObjsToPy(self.onlyWithout);
    }, [](PyGraphCreate &self, const py::object &obj) {
        self.onlyWithout = PyIniGraphGroups::modObjsFromPy(obj);
    }, py::doc(R"doc(
Optional[List[Tuple[:class:`str`, :class:`str`]]]: Objects -- one, or a list -- a graph of any of which, in ANY group, stops the edit: nothing is added when one
is there -- eg. a section that hides a target slot, added only when nothing is remapped onto that slot. **Default**: ``None``
    )doc"));

    cls.def_property("onlyWith", [](const PyGraphCreate &self) -> py::object {
        return PyIniGraphGroups::modObjsToPy(self.onlyWith);
    }, [](PyGraphCreate &self, const py::object &obj) {
        self.onlyWith = PyIniGraphGroups::modObjsFromPy(obj);
    }, py::doc(R"doc(
Optional[List[Tuple[:class:`str`, :class:`str`]]]: Objects -- one, or a list -- a graph of any of which, in ANY group, is what lets the edit add: nothing is
added when no group holds one -- eg. a texture list a fix writes for a component, added only when the mod draws that
component. The mirror of :attr:`onlyWithout`. **Default**: ``None``
    )doc"));

    cls.def_property("z3", py::cpp_function([](const PyGraphCreate &self) { return self.z3; }, py::return_value_policy::reference),
                     py::cpp_function([](PyGraphCreate &self, AGRC::Z3Context* z3) { self.z3 = z3; }, py::keep_alive<1, 2>()),
                     py::doc(R"doc(
Optional[:class:`Z3Context`]: The context a graph built from :attr:`sectionsOf` builds its conditions in, or ``None`` for its own. **Default**: ``None``
    )doc"));

    cls.def_readwrite("minimal", &PyGraphCreate::minimal, py::doc(R"doc(
:class:`bool`: Whether the copy taken of :attr:`graph` is a minimal one
    )doc"));

    cls.def_readwrite("newPartIds", &PyGraphCreate::newPartIds, py::doc(R"doc(
:class:`bool`: Whether the parts of the copy taken of :attr:`graph` get fresh ids
    )doc"));

    cls.def("edit", [](PyGraphCreate &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        (void)modType;
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
