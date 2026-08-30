#ifndef AGRemapPyBind_PyBaseIniGraphGroupEdit_H
#define AGRemapPyBind_PyBaseIniGraphGroupEdit_H

#include <string>

#include <pybind11/pybind11.h>

#include "PyIniGraphGroups.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::BaseIniGraphGroupEdit`\\<py::object, py::object\\>.
 Registered under the bare ``BaseIniGraphGroupEdit`` name; the pure-Python original this replaced
 has been removed :raw-html:`<br />` :raw-html:`<br />`

 A plain alias, not a subclass -- nothing on the C++ side ever holds a graph-group edit through
 this base and calls ``edit`` itself (its only consumer, ``GIMIFixer``, is still pure `Python`_),
 so no trampoline is needed: a `Python`_ subclass overriding ``edit`` is resolved by ordinary
 `Python`_ method lookup before anything reaches C++
 @endrst
 */
using PyBaseIniGraphGroupEdit = AGRC::BaseIniGraphGroupEdit<py::object, py::object, PyObjectHash, PyObjectEqual>;


/**
 * @brief
 @rst
 Resolves a `Python`_ ``(iniIndex, componentName, objectName)`` tuple into the
 :cpp:class:`AGRemapCore::BaseIniGraphGroupEdit::GraphId` every graph-group edit identifies a graph
 by :raw-html:`<br />` :raw-html:`<br />`

 A negative ``iniIndex`` maps to an index guaranteed to be out of range for any real group list, so
 it reads as "no such graph" rather than silently wrapping around the way a `Python`_ list index
 would -- the pure-Python original compared it with ``>= len(graphGroups)`` and never intended
 negative indexing either
 @endrst
 *
 * @param id The Python tuple to convert
 *
 * @throw pybind11::type_error If 'id' isn't a sequence of at least 3 elements
 */
PyBaseIniGraphGroupEdit::GraphId parseGraphId(const py::object &id);


/**
 * @brief
 @rst
 Converts a :cpp:class:`AGRemapCore::BaseIniGraphGroupEdit::GraphId` back into the
 ``(iniIndex, componentName, objectName)`` tuple `Python`_ uses
 @endrst
 *
 * @param id The graph id to convert
 */
py::tuple graphIdToPy(const PyBaseIniGraphGroupEdit::GraphId &id);


/**
 * @brief
 @rst
 Chains the shared ``getGraph``/``addGraph`` static bindings onto one graph-group edit's
 already-constructed ``py::class_`` :raw-html:`<br />` :raw-html:`<br />`

 These are ``classmethod``\\s on the pure-Python original, so every subclass sees them. Real
 `pybind11`_ inheritance already hands subclasses the base's statics, so in practice only the base
 itself calls this -- it stays a helper (rather than being inlined into
 :cpp:func:`initCppBaseIniGraphGroupEdit`) so a future graph-group edit that ends up *not*
 inheriting this base can reuse the exact same pair, docstrings included
 @endrst
 *
 * @tparam PyClass The concrete ``py::class_`` type being extended
 *
 * @param cls The class to chain the bindings onto
 */
template <typename PyClass>
void bindGraphGroupEditLookupMethods(PyClass &cls) {
    cls.def_static("getGraph", [](py::list graphGroups, const py::object &id, bool errorOnNotFound, py::object defaultVal) -> py::object {
        PyIniGraphGroups groups(std::move(graphGroups));
        PyBaseIniGraphGroupEdit::GraphId graphId = parseGraphId(id);

        // Deliberately asks the core for the no-throw answer and raises here instead: the core
        // throws std::out_of_range (which pybind11 surfaces as IndexError), while the pure-Python
        // original raised KeyError and real callers catch that exact class.
        PyBaseIniGraphGroupEdit::Graph *result = PyBaseIniGraphGroupEdit::getGraph(groups, graphId, false);

        if (result == nullptr && errorOnNotFound) {
            throw py::key_error("No .ini graph found by the key: " + py::str(id).cast<std::string>());
        }

        if (result == nullptr) {
            return defaultVal;
        }

        return groups.graphToPy(result);
    }, py::arg("graphGroups"), py::arg("id"), py::arg("errorOnNotFound") = true, py::arg("default") = py::none(),
       py::doc(R"doc(
Retrieves the corresponding graph from a group of graphs

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

id: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id to retrieve the graph. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

errorOnNotFound: :class:`bool`
    If no graphs are found, whether to raise an exception :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

default: Any
    If 'errorOnNotFound' is ``False``, then the default value to return if no graphs are found :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Raises
------
`KeyError`_
    If no graphs are found

Returns
-------
Union[:class:`IniSectionGraph`, Any]
    Either the found graph or the value specified at 'default', if no graphs were found and
    'errorOnNotFound' is set to ``False``
       )doc"));

    cls.def_static("addGraph", [](py::list graphGroups, const py::object &id, py::object graph) {
        PyIniGraphGroups groups(std::move(graphGroups));
        PyBaseIniGraphGroupEdit::GraphId graphId = parseGraphId(id);

        // 'graph' is a graph this view has never seen (it comes straight from the caller), so it
        // has to be registered before the core-facing pointer means anything -- see
        // PyIniGraphGroups::adopt.
        PyBaseIniGraphGroupEdit::Graph *parsedGraph = groups.adopt(std::move(graph));
        return PyBaseIniGraphGroupEdit::addGraph(groups, graphId, parsedGraph);
    }, py::arg("graphGroups"), py::arg("id"), py::arg("graph"), py::doc(R"doc(
Adds a graph to the group of graphs

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

id: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id for where to add the graph. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

graph: :class:`IniSectionGraph`
    The graph to add

Returns
-------
:class:`bool`
    Whether the graph has been added
    )doc"));
}


/**
 * @brief Registers the Python-facing ``BaseIniGraphGroupEdit``
 *
 * @param m The module to register into
 */
void initCppBaseIniGraphGroupEdit(pybind11::module_ &m);

#endif
