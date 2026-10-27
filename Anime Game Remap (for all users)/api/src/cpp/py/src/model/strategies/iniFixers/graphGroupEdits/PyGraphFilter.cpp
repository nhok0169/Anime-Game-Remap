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

#include "PyGraphFilter.h"

#include <memory>
#include <utility>
#include <vector>

#include "PyIniGraphGroups.h"
#include "../../../iftemplate/PyIfContentPart.h"


AGRC::GraphFilter<std::string, std::string>::Predicate toGraphPredicate(const py::object &predicate) {
    if (predicate.is_none()) {
        return nullptr;
    }
    return [predicate](const std::string &sectionName, const PyIfContentPart &part, std::size_t index) {
        return predicate(sectionName, py::cast(&part, py::return_value_policy::reference), index).cast<bool>();
    };
}


PyGraphFilter::PyGraphFilter(py::object graphIdsObj, py::object predicateObj):
    Core({}, nullptr), graphIdsObj(std::move(graphIdsObj)), predicateObj(std::move(predicateObj)) {}


void PyGraphFilter::refresh() {
    std::vector<GraphId> parsed;
    if (!graphIdsObj.is_none()) {
        for (auto item : graphIdsObj) {
            parsed.push_back(parseGraphId(py::reinterpret_borrow<py::object>(item)));
        }
    }
    graphIds = std::move(parsed);
    predicate = toGraphPredicate(predicateObj);
}


void initCppGraphFilter(pybind11::module_ &m) {
    py::class_<PyGraphFilter, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphFilter", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Keeps only part of some graphs, in place: every `KVP`_ that :attr:`predicate` rejects is removed
from the graphs' `sections`_

The `section`_'s ``if`` / ``else`` structure is kept as it is, so a `KVP`_ that survives stays under
the same condition it was written under. :class:`GraphCopy` makes the same selection on a COPY of a
graph instead

.. note::
    A `section`_ a removed ``run =`` used to call stays in the graph: the selection is made per
    `KVP`_, and what the graph reaches is not recomputed

Parameters
----------
graphIds: List[Tuple[:class:`int`, :class:`str`, :class:`str`]]
    The ids of the graphs to filter: the .ini index, the component name and the mod object name

predicate: Optional[Callable[[:class:`str`, :class:`IfContentPart`, :class:`int`], :class:`bool`]]
    Decides whether one `KVP`_ is kept, given the name of its `section`_, the part holding it and its
    index within that part. Every decision is made before anything is removed, so the predicate
    always sees the part as it was. ``None`` keeps everything :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object graphIds, py::object predicate) {
        return std::make_unique<PyGraphFilter>(std::move(graphIds), std::move(predicate));
    }), py::arg("graphIds"), py::arg("predicate") = py::none());

    cls.def_property("graphIds", [](const PyGraphFilter &self) { return self.graphIdsObj; },
                     [](PyGraphFilter &self, py::object v) { self.graphIdsObj = std::move(v); }, py::doc(R"doc(
List[Tuple[:class:`int`, :class:`str`, :class:`str`]]: The ids of the graphs to filter
    )doc"));

    cls.def_property("predicate", [](const PyGraphFilter &self) { return self.predicateObj; },
                     [](PyGraphFilter &self, py::object v) { self.predicateObj = std::move(v); }, py::doc(R"doc(
Optional[Callable[[:class:`str`, :class:`IfContentPart`, :class:`int`], :class:`bool`]]: Decides which `KVPs`_ are kept
    )doc"));

    cls.def("edit", [](PyGraphFilter &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        (void)modType;
        self.refresh();
        PyIniGraphGroups groups(graphGroups);
        // nullptr for the same reason GraphRemove's binding passes it -- this edit never reads it
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Removes, from every graph named by :attr:`graphIds`, the `KVPs`_ :attr:`predicate` rejects

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
    The same list that was passed in, after filtering
    )doc"));
}
