#include "PyIniGraphGroup.h"

#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace py = pybind11;


PyIniGraphGroup::PyIniGraphGroup(py::dict graphs): graphs(std::move(graphs)) {
}

void PyIniGraphGroup::addGraph(py::object modObj, py::object graph) {
    graphs[modObj] = graph;
}

py::object PyIniGraphGroup::removeGraph(py::object modObj) {
    return graphs.attr("pop")(modObj, py::none());
}

std::string PyIniGraphGroup::toStr(bool autoindent) const {
    std::ostringstream result;
    bool first = true;

    for (auto item : graphs) {
        py::object graph = py::reinterpret_borrow<py::object>(item.second);
        std::string currentResult = graph.attr("toStr")(py::arg("autoindent") = autoindent).cast<std::string>();
        if (currentResult.empty()) {
            continue;
        }

        if (!first) {
            result << "\n\n";
        }
        result << currentResult;
        first = false;
    }

    return result.str();
}


void initCppIniGraphGroup(pybind11::module_ &m) {
    py::class_<PyIniGraphGroup>(m, "IniGraphGroup", R"doc(
A class to represent a group of caller/callee graphs within a .ini file
    )doc")

        // py::init(factory), not a plain 'py::arg("graphs") = py::dict()' default -- that form
        // bakes in ONE Python dict object at bind time, shared by every construction that omits
        // 'graphs' from then on (pybind11's version of Python's own mutable-default-argument
        // footgun -- see PyIniGroupedResource.cpp's identical fix/comment for the full story). A
        // lambda constructs a genuinely fresh py::dict() each time it actually runs.
        .def(py::init([](py::object graphs) {
            py::dict graphsDict = graphs.is_none() ? py::dict() : graphs.cast<py::dict>();
            return std::make_unique<PyIniGraphGroup>(std::move(graphsDict));
        }), py::arg("graphs") = py::none(), py::doc(R"doc(
Constructs a new group of graphs

Parameters
----------
graphs: Optional[Dict[Tuple[:class:`str`, :class:`str`], :class:`IniSectionGraph`]]
    The group of graphs -- the keys contain the name of the component and the name of the mod
    object, and the values are the associated graph. If ``None``, a fresh empty ``dict`` is used

    **Default**: ``None``
        )doc"))

        .def_readwrite("graphs", &PyIniGraphGroup::graphs, py::doc(R"doc(
Dict[Tuple[:class:`str`, :class:`str`], :class:`IniSectionGraph`]: The group of graphs -- the keys
contain the name of the component and the name of the mod object, and the values are the associated
graph
        )doc"))

        .def("addGraph", &PyIniGraphGroup::addGraph, py::arg("modObj"), py::arg("graph"), py::doc(R"doc(
Adds a new graph

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The associated component and mod object for the graph

graph: :class:`IniSectionGraph`
    The new graph to add
        )doc"))

        .def("removeGraph", &PyIniGraphGroup::removeGraph, py::arg("modObj"), py::doc(R"doc(
Removes a graph based on the specified component and mod object

Parameters
----------
modObj: Tuple[:class:`str`, :class:`str`]
    The name of the component and mod object

Returns
-------
Optional[:class:`IniSectionGraph`]
    The associated graph, if removed
        )doc"))

        .def("toStr", &PyIniGraphGroup::toStr, py::arg("autoindent") = true, py::doc(R"doc(
Converts all the sections in the group of graphs to a string

Parameters
----------
autoindent: :class:`bool`
    Whether to compute the proper tab indent for the section

    **Default**: ``True``

Returns
-------
:class:`str`
    The string representation
        )doc"));
}
