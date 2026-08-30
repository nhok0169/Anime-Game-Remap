#include "PyGraphRemove.h"

#include <memory>
#include <utility>
#include <vector>


PyGraphRemove::PyGraphRemove(py::object graphIdsObj): Core({}), graphIdsObj(std::move(graphIdsObj)) {}


void PyGraphRemove::refresh() {
    std::vector<GraphId> parsed;

    if (!graphIdsObj.is_none()) {
        for (auto item : graphIdsObj) {
            parsed.push_back(parseGraphId(py::reinterpret_borrow<py::object>(item)));
        }
    }

    graphIds = std::move(parsed);
}


void initCppGraphRemove(pybind11::module_ &m) {
    py::class_<PyGraphRemove, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphRemove", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Removes some graphs from a group of graphs

.. note::
    A graph id that names no existing graph (a missing ``(component, object)`` key, or an
    out-of-range .ini index) is skipped silently -- no exception

Parameters
----------
graphIds: List[Tuple[:class:`int`, :class:`str`, :class:`str`]]
    The ids of the graphs to remove. Each tuple contains: :raw-html:`<br />` :raw-html:`<br />`

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object
    )doc");

    // py::init(factory) rather than py::init<py::object>(): the core class owns std::function
    // members through its base's typedefs, and a factory returning a unique_ptr avoids ever
    // needing to move-construct the class itself -- see PyRegAdd.cpp's identical note.
    cls.def(py::init([](py::object graphIds) {
        return std::make_unique<PyGraphRemove>(std::move(graphIds));
    }), py::arg("graphIds"));

    cls.def_property("graphIds", [](const PyGraphRemove &self) {
        return self.graphIdsObj;
    }, [](PyGraphRemove &self, py::object graphIds) {
        self.graphIdsObj = std::move(graphIds);
    }, py::doc(R"doc(
List[Tuple[:class:`int`, :class:`str`, :class:`str`]]: The ids of the graphs to remove
    )doc"));

    cls.def("edit", [](PyGraphRemove &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        self.refresh();

        PyIniGraphGroups groups(graphGroups);
        // The C++ core takes 'modType' as a nullable ModType*, and the Python-side ModType is a
        // pure-Python class with no C++ counterpart to cast to, so nullptr is the only honest
        // thing to pass -- this edit never reads it anyway.
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Removes every graph named by :attr:`graphIds` from 'graphGroups'

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
    The same list that was passed in, after the graphs were removed
    )doc"));
}
