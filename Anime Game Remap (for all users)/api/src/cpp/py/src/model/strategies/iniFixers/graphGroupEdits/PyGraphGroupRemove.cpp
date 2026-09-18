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

#include "PyGraphGroupRemove.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>


PyGraphGroupRemove::PyGraphGroupRemove(py::object iniIndicesObj): Core(std::nullopt), iniIndicesObj(std::move(iniIndicesObj)) {}


void PyGraphGroupRemove::refresh() {
    if (iniIndicesObj.is_none()) {
        iniIndices = std::nullopt;
        return;
    }

    std::vector<std::size_t> parsed;
    for (auto item : iniIndicesObj) {
        const long long index = py::cast<long long>(item);

        // A negative index names no group, and is skipped the way an index past the end is.
        if (index >= 0) {
            parsed.push_back(static_cast<std::size_t>(index));
        }
    }

    iniIndices = std::move(parsed);
}


void initCppGraphGroupRemove(pybind11::module_ &m) {
    py::class_<PyGraphGroupRemove, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphGroupRemove", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Removes whole groups of graphs --- every graph one .ini file of the fix would be rendered from ---
where :class:`GraphRemove` removes single graphs out of a group

Removing EVERY group is how a fixer that has given up writes nothing. A fixer renders every graph
the parser handed it whether or not an edit touched it, so a fixer that simply stops building its
edits writes the mod's own `sections`_ out again after the remap header, under the SOURCE's names
--- which the remover cannot tell from the author's, so every later run appends another copy

.. note::
    A group index past the end (or a negative one) is skipped silently, as :class:`GraphRemove`
    skips an out-of-range graph id. Groups are removed from the highest index down, so the indices
    given all mean the groups as they were before this edit ran

Parameters
----------
iniIndices: Optional[List[:class:`int`]]
    The .ini indices of the groups to remove, or ``None`` for every group :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object iniIndices) {
        return std::make_unique<PyGraphGroupRemove>(std::move(iniIndices));
    }), py::arg("iniIndices") = py::none());

    cls.def_property("iniIndices", [](const PyGraphGroupRemove &self) {
        return self.iniIndicesObj;
    }, [](PyGraphGroupRemove &self, py::object iniIndices) {
        self.iniIndicesObj = std::move(iniIndices);
    }, py::doc(R"doc(
Optional[List[:class:`int`]]: The .ini indices of the groups to remove, or ``None`` for every group
    )doc"));

    cls.def("edit", [](PyGraphGroupRemove &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        (void)modType;
        self.refresh();

        PyIniGraphGroups groups(graphGroups);

        // nullptr for the same reason GraphRemove's binding passes it -- this edit never reads it.
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Removes every group named by :attr:`iniIndices` from 'graphGroups'

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
    The same list that was passed in, after the groups were removed
    )doc"));
}
