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

#include <string>
#include <utility>

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ParseDataStep.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"

#include "PyBaseIniGraphGroupEdit.h"
#include "PyIniGraphGroups.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppParseDataStep(pybind11::module_ &m) {
    using Step = AGRC::ParseDataStep;
    auto cls = py::class_<Step, PyBaseIniGraphGroupEdit, AGRC::IParseResult, py::smart_holder>(m, "ParseDataStep", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit` and :class:`IParseResult`

A parse step that edits nothing and, when the parser files it, calls a function with the parser's
:class:`IniParseData`

Put it in :attr:`GIMIParser.parseSteps` or :attr:`GIMIParser.deferredParseSteps` to read what the steps before it
filed -- the rows of a :class:`GraphCollect`, say -- and file a conclusion of your own, in the order the steps run. A
later step sees what this one filed. The function is handed the parser's own :class:`IniParseData`, not a copy, so
what it changes stays

Parameters
----------
file: Optional[Callable[[:class:`IniParseData`], None]]
    The function called with the parser's data when the step is filed; ``None`` calls nothing
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Examples
--------

.. code-block:: python

    import FixRaidenBoss2 as FRB

    collect = FRB.GraphCollect({(0, "", "body"): ["ps-t0"]}, name = "body")
    seen = []
    step = FRB.ParseDataStep(lambda data: seen.extend(row.value for row in data.collected("body")))
    parser = FRB.GIMIParser(iniFile, modObjs = [("", "body")], parseSteps = [collect, step])
    )doc");

    cls.def(py::init([](py::object file) {
        if (file.is_none()) {
            return std::make_unique<Step>(Step::File());
        }

        // the parser's own data, by reference: a copy would lose what the function files
        return std::make_unique<Step>([file = std::move(file)](AGRC::IniParseData& data) {
            py::gil_scoped_acquire gil;
            file(py::cast(&data, py::return_value_policy::reference));
        });
    }), py::arg("file") = py::none());

    cls.def("edit", [](Step& self, py::list graphGroups, const py::object& modType, const std::string& modName) {
        (void)modType;
        PyIniGraphGroups groups(graphGroups);
        self.edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Edits nothing

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in
    )doc"));

    cls.def("editFromIni", [](Step& self, py::list graphGroups, const py::object& ini, const py::object& modType,
                              const std::string& modName) {
        (void)ini;
        (void)modType;
        PyIniGraphGroups groups(graphGroups);
        self.edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Edits nothing

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in
    )doc"));
}
