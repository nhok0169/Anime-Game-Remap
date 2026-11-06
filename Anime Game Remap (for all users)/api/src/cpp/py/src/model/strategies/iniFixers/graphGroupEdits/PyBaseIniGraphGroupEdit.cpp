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

#include "PyBaseIniGraphGroupEdit.h"

#include <limits>
#include <utility>

#include "AGRemapCore/model/strategies/iniFixers/BaseIniPartEdit.h"
#include "AGRemapCore/model/files/IniFile.h"


PyBaseIniGraphGroupEdit::GraphId parseGraphId(const py::object &id) {
    if (!py::isinstance<py::sequence>(id) || py::isinstance<py::str>(id)) {
        throw py::type_error("A graph id must be a (iniIndex, componentName, objectName) tuple");
    }

    py::sequence idSeq = py::reinterpret_borrow<py::sequence>(id);
    if (idSeq.size() < 3) {
        throw py::type_error("A graph id must be a (iniIndex, componentName, objectName) tuple");
    }

    PyBaseIniGraphGroupEdit::GraphId result;
    long long iniIndex = py::reinterpret_borrow<py::object>(idSeq[0]).cast<long long>();

    // See this function's own doc comment -- a negative .ini index is "no such graph", not a
    // Python-style index from the end.
    result.iniIndex = (iniIndex < 0) ? std::numeric_limits<std::size_t>::max() : static_cast<std::size_t>(iniIndex);
    result.modObj = PyBaseIniGraphGroupEdit::ModObj(py::str(idSeq[1]).cast<std::string>(),
                                                     py::str(idSeq[2]).cast<std::string>());
    return result;
}


py::tuple graphIdToPy(const PyBaseIniGraphGroupEdit::GraphId &id) {
    return py::make_tuple(py::cast(id.iniIndex), py::str(id.modObj.first), py::str(id.modObj.second));
}


void initCppBaseIniGraphGroupEdit(pybind11::module_ &m) {
    auto cls = py::class_<PyBaseIniGraphGroupEdit, AGRC::BaseIniPartEdit, py::smart_holder>(m, "BaseIniGraphGroupEdit", R"doc(
This class inherits from :class:`BaseIniPartEdit`

Base class for a filter that edits a group of caller/callee graphs across many .ini files
    )doc")

        .def(py::init<>())

        // Deliberately routed through self.attr("edit") rather than bound to the C++
        // BaseIniGraphGroupEdit::editFromIni (which calls this->edit(...) as a *C++* virtual
        // call): with no trampoline in play, a pure-Python subclass overriding only 'edit' has no
        // C++-side vtable entry for that override, so a C++-internal virtual call would silently
        // run the no-op base implementation. See PyBaseRegEdit.cpp's identical comment.
        .def("editFromIni", [](py::object self, py::list graphGroups, const py::object &ini,
                               const py::object &modType, const std::string &modName) {
            // An edit built in C++ whose class has no Python class of its own (the standard rows
            // GIMIParser::rowSteps builds) reaches Python as this base: it runs its own C++ editFromIni,
            // handed the core IniFile, as it would from C++
            if (py::type::of(self).is(py::type::of<PyBaseIniGraphGroupEdit>())) {
                AGRC::IniFile* coreIni = py::isinstance<AGRC::IniFile>(ini) ? ini.cast<AGRC::IniFile*>() : nullptr;
                PyIniGraphGroups groups(graphGroups);
                self.cast<PyBaseIniGraphGroupEdit&>().editFromIni(groups, coreIni, nullptr, modName);
                return py::object(std::move(graphGroups));
            }

            // Otherwise 'ini' is unused -- exactly as in the pure-Python original.
            return self.attr("edit")(std::move(graphGroups), modType, modName);
        }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Edits a group of caller/callee graphs with state info from 'ini'

.. note::
    This forwards straight to :meth:`edit` and ignores 'ini' entirely

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

ini: :class:`IniFile`
    The associated original .ini file

modType: :class:`ModType`
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The resultant group of graphs that got editted
        )doc"))

        .def("edit", [](py::object self, py::list graphGroups, const py::object &modType, const std::string &modName) {
            (void)modType;
            // an edit built in C++ runs its own C++ edit (see editFromIni)
            if (py::type::of(self).is(py::type::of<PyBaseIniGraphGroupEdit>())) {
                PyIniGraphGroups groups(graphGroups);
                self.cast<PyBaseIniGraphGroupEdit&>().edit(groups, nullptr, modName);
            }
            return graphGroups;
        }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Edits a group of caller/callee graphs

.. note::
    The base implementation is a no-op that hands 'graphGroups' straight back

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modType: :class:`ModType`
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The resultant group of graphs that got editted
        )doc"));

    bindGraphGroupEditLookupMethods(cls);
}
