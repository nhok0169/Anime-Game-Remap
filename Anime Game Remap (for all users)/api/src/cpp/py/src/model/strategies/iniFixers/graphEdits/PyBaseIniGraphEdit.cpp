#include "PyBaseIniGraphEdit.h"

#include <string>
#include <utility>

#include "AGRemapCore/model/strategies/iniFixers/BaseIniGraphPartEdit.h"


PyIniSectionGraph &parseGraphArg(const py::object &graph) {
    if (!py::isinstance<PyIniSectionGraph>(graph)) {
        throw py::type_error("A graph edit's 'graph' argument must be an IniSectionGraph");
    }

    return py::cast<PyIniSectionGraph &>(graph);
}


void initCppBaseIniGraphEdit(pybind11::module_ &m) {
    py::class_<PyBaseIniGraphEdit, AGRC::BaseIniGraphPartEdit, py::smart_holder>(m, "BaseIniGraphEdit", R"doc(
This class inherits from :class:`CppBaseIniGraphPartEdit`

Base class for a filter that edits some caller/callee graph of :class:`IniSectionGraph`
    )doc")

        .def(py::init<>())

        // Deliberately routed through self.attr("edit") rather than bound to the C++
        // BaseIniGraphEdit::editFromIni (which calls this->edit(...) as a *C++* virtual call): with
        // no trampoline in play, a pure-Python subclass overriding only 'edit' has no C++-side
        // vtable entry for that override, so a C++-internal virtual call would silently run the
        // no-op base implementation. See PyBaseRegEdit.cpp's identical comment.
        .def("editFromIni", [](py::object self, py::object graph, const py::object &ini,
                               const py::object &modType, const std::string &modName,
                               const py::object &partFilter, bool trackKeys, const py::object &keysToTrack) {
            // 'ini' is deliberately unused -- exactly as in the pure-Python original.
            (void)ini;
            return self.attr("edit")(std::move(graph), modType, py::arg("modName") = modName,
                                     py::arg("partFilter") = partFilter, py::arg("trackKeys") = trackKeys,
                                     py::arg("keysToTrack") = keysToTrack);
        }, py::arg("graph"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "",
           py::arg("partFilter") = py::none(), py::arg("trackKeys") = false,
           py::arg("keysToTrack") = py::none(), py::doc(R"doc(
Edits the caller/callee graph of :class:`IniSectionGraph` with state info from 'ini'

.. note::
    This forwards straight to :meth:`edit` and ignores 'ini' entirely, exactly as the pure-Python
    original does

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to edit

ini: Optional[:class:`IniFile`]
    The associated .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
    The filter used to indicate the valid order indices to process some :class:`IfContentPart` in
    the graph :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

trackKeys: :class:`bool`
    The **caller's** key-tracking default, handed down by whatever is driving this edit
    (:class:`GraphGroupEdit` passes its own ``trackKeys`` here) :raw-html:`<br />` :raw-html:`<br />`

    A subclass with its own key-tracking setting decides how to combine the two; a subclass without
    one simply ignores it :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[Set[:class:`str`]]
    The **caller's** key-tracking key set, handed down the same way. ``None`` means every key
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IniSectionGraph`
    The resultant graph that got editted
        )doc"))

        .def("edit", [](PyBaseIniGraphEdit &self, py::object graph, const py::object &modType,
                        const std::string &modName, const py::object &partFilter, bool trackKeys,
                        const py::object &keysToTrack) {
            (void)self;
            (void)modType;
            (void)modName;
            (void)partFilter;
            (void)trackKeys;
            (void)keysToTrack;
            return graph;
        }, py::arg("graph"), py::arg("modType"), py::arg("modName") = "",
           py::arg("partFilter") = py::none(), py::arg("trackKeys") = false,
           py::arg("keysToTrack") = py::none(), py::doc(R"doc(
Edits the caller/callee graph of :class:`IniSectionGraph`

.. note::
    The base implementation is a no-op that hands 'graph' straight back, matching the pure-Python
    original's ``pass``

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to edit

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
    The filter used to indicate the valid order indices to process some :class:`IfContentPart` in
    the graph :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

trackKeys: :class:`bool`
    The **caller's** key-tracking default, handed down by whatever is driving this edit
    (:class:`GraphGroupEdit` passes its own ``trackKeys`` here) :raw-html:`<br />` :raw-html:`<br />`

    A subclass with its own key-tracking setting decides how to combine the two; a subclass without
    one simply ignores it :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[Set[:class:`str`]]
    The **caller's** key-tracking key set, handed down the same way. ``None`` means every key
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IniSectionGraph`
    The resultant graph that got editted
        )doc"));
}
