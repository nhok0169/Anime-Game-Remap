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

#include "PyIterGraphGroupEdit.h"

#include <memory>
#include <utility>


PyIterGraphGroupEdit::PyIterGraphGroupEdit(py::object makeEditsObj): Core(nullptr), makeEditsObj(std::move(makeEditsObj)) {}


py::list PyIterGraphGroupEdit::run(py::list graphGroups, const py::object &ini, const py::object &modType, const std::string &modName,
                                   bool fromIni) {
    if (makeEditsObj.is_none()) {
        return graphGroups;
    }

    // counted once, before the first group is edited, as the core class does
    const std::size_t graphGroupsLen = py::len(graphGroups);
    for (std::size_t iniIndex = 0; iniIndex < graphGroupsLen; ++iniIndex) {
        py::object edits = makeEditsObj(iniIndex, graphGroups);
        if (edits.is_none()) {
            continue;
        }

        for (auto edit : edits) {
            if (edit.is_none()) {
                continue;
            }

            if (fromIni) {
                edit.attr("editFromIni")(graphGroups, ini, modType, py::arg("modName") = modName);
            } else {
                edit.attr("edit")(graphGroups, modType, py::arg("modName") = modName);
            }
        }
    }

    return graphGroups;
}


void initCppIterGraphGroupEdit(pybind11::module_ &m) {
    py::class_<PyIterGraphGroupEdit, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "IterGraphGroupEdit", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Runs the edits a factory builds for each group of graphs, on every group there is when the edit runs

What it is for: edits on graphs that only exist at fix time. :class:`GraphGroupEdit` is handed its edits
one .ini file at a time when it is built, but an edit such as :class:`GraphGroupRemap` decides at fix time
how many .ini files there are, and an edit that names a graph -- a :class:`ResRegCollect`'s keys, for
example -- names it by its .ini file's index. The factory is handed that index and the groups, and builds
the group's edits from them

.. note::
    The factory is called for **every** group, so the edits it builds have to be right on every group: an
    edit that would do something wrong on some group must not be returned for it, or must carry its own
    filter that makes it a no-op there. To edit chosen groups only, use :class:`GraphGroupEdit`

Parameters
----------
makeEdits: Optional[Callable[[:class:`int`, List[:class:`IniGraphGroup`]], Optional[List[:class:`BaseIniGraphGroupEdit`]]]]
    The factory that builds each group's edits: it takes the group's .ini file index and the groups, and
    returns the edits to run, in order, or ``None`` to leave the group alone
    )doc");

    cls.def(py::init([](py::object makeEdits) {
        return std::make_unique<PyIterGraphGroupEdit>(std::move(makeEdits));
    }), py::arg("makeEdits") = py::none());

    cls.def_property("makeEdits", [](const PyIterGraphGroupEdit &self) {
        return self.makeEditsObj;
    }, [](PyIterGraphGroupEdit &self, py::object makeEdits) {
        self.makeEditsObj = std::move(makeEdits);
    }, py::doc(R"doc(
Optional[Callable[[:class:`int`, List[:class:`IniGraphGroup`]], Optional[List[:class:`BaseIniGraphGroupEdit`]]]]: The factory that builds each group's edits
    )doc"));

    cls.def("edit", [](PyIterGraphGroupEdit &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        return self.run(std::move(graphGroups), py::none(), modType, modName, false);
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Calls :attr:`makeEdits` for each group, in order of the .ini file index, and runs the edits it returns
with their ``edit``

The groups are counted once, before the first one is edited, so a group an edit adds is not visited

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after editing
    )doc"));

    cls.def("editFromIni", [](PyIterGraphGroupEdit &self, py::list graphGroups, const py::object &ini, const py::object &modType,
                              const std::string &modName) {
        return self.run(std::move(graphGroups), ini, modType, modName, true);
    }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
The same as :meth:`edit`, running each edit with its ``editFromIni``

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

ini: Optional[:class:`IniFile`]
    The associated original .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after editing
    )doc"));
}
