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

#include "PyGraphCopy.h"

#include <memory>
#include <utility>

#include "PyGraphFilter.h"
#include "PyIniGraphGroups.h"


PyGraphCopy::PyGraphCopy(py::object srcIdObj, py::object dstIdObj, py::object predicateObj, bool minimal, bool newPartIds):
    Core(Core::GraphId(), Core::GraphId(), nullptr, minimal, newPartIds),
    srcIdObj(std::move(srcIdObj)), dstIdObj(std::move(dstIdObj)), predicateObj(std::move(predicateObj)) {}


void PyGraphCopy::refresh() {
    srcId = parseGraphId(srcIdObj);
    dstId = parseGraphId(dstIdObj);
    predicate = toGraphPredicate(predicateObj);
}


void initCppGraphCopy(pybind11::module_ &m) {
    py::class_<PyGraphCopy, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphCopy", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Copies one graph of a group of graphs to another graph id --- optionally only the part of it
:attr:`predicate` keeps (the same selection :class:`GraphFilter` makes in place)

The copy is a deep copy, so editing it never touches the source. Its `sections`_ keep the source's
names: rename them (:class:`GraphRename`) before both graphs are rendered into one file. To compose
the copy into another graph, follow with :class:`GraphInherit`

.. note::
    The destination follows :class:`GraphCreate`'s rule: an ``iniIndex`` equal to the number of
    groups appends one fresh group, one further past the end is skipped, and an existing graph at
    the destination is replaced. A source id naming no graph copies nothing

Parameters
----------
srcId: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id of the graph to copy: the .ini index, the component name and the mod object name

dstId: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id the copy is stored under

predicate: Optional[Callable[[:class:`IfTemplate`, :class:`IfContentPart`, :class:`int`], :class:`bool`]]
    Decides which `KVPs`_ of the copy are kept --- see :attr:`GraphFilter.predicate`. ``None`` copies
    the whole graph :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

minimal: :class:`bool`
    Whether the copy is a minimal one :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

newPartIds: :class:`bool`
    Whether the parts of the copy get fresh ids :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc");

    cls.def(py::init([](py::object srcId, py::object dstId, py::object predicate, bool minimal, bool newPartIds) {
        return std::make_unique<PyGraphCopy>(std::move(srcId), std::move(dstId), std::move(predicate), minimal, newPartIds);
    }), py::arg("srcId"), py::arg("dstId"), py::arg("predicate") = py::none(), py::arg("minimal") = true, py::arg("newPartIds") = true);

    cls.def_property("srcId", [](const PyGraphCopy &self) { return self.srcIdObj; },
                     [](PyGraphCopy &self, py::object v) { self.srcIdObj = std::move(v); }, py::doc(R"doc(
Tuple[:class:`int`, :class:`str`, :class:`str`]: The id of the graph to copy
    )doc"));

    cls.def_property("dstId", [](const PyGraphCopy &self) { return self.dstIdObj; },
                     [](PyGraphCopy &self, py::object v) { self.dstIdObj = std::move(v); }, py::doc(R"doc(
Tuple[:class:`int`, :class:`str`, :class:`str`]: The id the copy is stored under
    )doc"));

    cls.def_property("predicate", [](const PyGraphCopy &self) { return self.predicateObj; },
                     [](PyGraphCopy &self, py::object v) { self.predicateObj = std::move(v); }, py::doc(R"doc(
Optional[Callable[[:class:`IfTemplate`, :class:`IfContentPart`, :class:`int`], :class:`bool`]]: Decides which `KVPs`_ of the copy are kept
    )doc"));

    cls.def_readwrite("minimal", &PyGraphCopy::minimal, py::doc(R"doc(
:class:`bool`: Whether the copy is a minimal one
    )doc"));

    cls.def_readwrite("newPartIds", &PyGraphCopy::newPartIds, py::doc(R"doc(
:class:`bool`: Whether the parts of the copy get fresh ids
    )doc"));

    cls.def("edit", [](PyGraphCopy &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        (void)modType;
        self.refresh();
        PyIniGraphGroups groups(graphGroups);
        // nullptr for the same reason GraphRemove's binding passes it -- this edit never reads it
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Stores a (filtered) copy of the graph at :attr:`srcId` under :attr:`dstId`

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
    The same list that was passed in, after the copy was added
    )doc"));
}
