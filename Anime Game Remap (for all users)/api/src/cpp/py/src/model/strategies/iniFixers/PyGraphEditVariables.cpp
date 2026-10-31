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

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/iniFixers/GraphEditVariables.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/VarSectionsCreate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/VarSectionsTake.h"
#include "AGRemapCore/tools/z3/Z3Context.h"

#include "graphGroupEdits/PyBaseIniGraphGroupEdit.h"
#include "graphGroupEdits/PyIniGraphGroups.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {
    // 'edit' and 'editFromIni' over a list of IniGraphGroup, for an edit whose C++ 'edit' does the work
    template <typename Edit, typename Cls>
    void defEdit(Cls& cls, const char* what) {
        cls.def("edit", [](Edit& self, py::list graphGroups, const py::object& modType, const std::string& modName) {
            (void)modType;
            PyIniGraphGroups groups(graphGroups);
            self.edit(groups, nullptr, modName);
            return graphGroups;
        }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(what));

        cls.def("editFromIni", [](Edit& self, py::list graphGroups, const py::object& ini, const py::object& modType,
                                  const std::string& modName) {
            (void)ini;
            (void)modType;
            PyIniGraphGroups groups(graphGroups);
            self.edit(groups, nullptr, modName);
            return graphGroups;
        }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(what));
    }

    const char* const EditReturns = R"doc(
Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in
)doc";
}


void initCppGraphEditVariables(pybind11::module_ &m) {
    using Vars = AGRC::GraphEditVariables;
    using Sections = Vars::Sections;

    auto vars = py::class_<Vars>(m, "GraphEditVariables", R"doc(
Named variables that edits save data about one graph into, and read or write back out later -- the data a list of
edits passes from one edit to another

A variable holds one graph's worth of `sections`_ (:class:`GraphEditVariables.Sections`): an edit copies sections of a
graph into it (:class:`VarSectionsTake`), code adds sections it builds itself (:meth:`GraphEditVariables.Sections.add`),
and an edit writes it out as a graph (:class:`VarSectionsCreate`). A fixer's own sections are one such variable

.. warning::
    A variable is a snapshot. An edit that changes a graph afterwards does not change what a variable copied from it
    holds, and an edit that changes the graph a variable was written to does not change the variable

:example:

.. code-block:: python

    import FixRaidenBoss2 as FRB

    variables = FRB.GraphEditVariables()
    variables.sections("fix").add(FRB.IfTemplate([FRB.IfContentPart({"handling": [(0, "skip")]}, 0)], name = "TextureOverrideMine"))
    take = FRB.VarSectionsTake(variables, "fix", ("", "body"), ["TextureOverrideBody"])
    create = FRB.VarSectionsCreate(variables, ("", "fixSections"), lambda group: ["fix"] if group == 0 else [])
    fixer.graphGroupEdits = [take, create]
    )doc");

    py::class_<Sections>(vars, "Sections", R"doc(
A variable holding the `sections`_ of one graph, in order

Each :meth:`add` is one place in the order. :meth:`reserve` holds a place open so sections known only later land where
the reservation was made rather than after everything added since
    )doc")
        .def("add", [](Sections& self, const Vars::Section& section) { self.add(section.deepcopy()); }, py::arg("section"),
             py::doc(R"doc(
Adds a copy of a section at the end

Parameters
----------
section: :class:`IfTemplate`
    The section
             )doc"))
        .def("reserve", &Sections::reserve, py::doc(R"doc(
Holds a place open at the end, for sections added later with :meth:`addAt`

Returns
-------
Tuple[:class:`int`, :class:`int`]
    The place: its index, and how many times the variable was cleared when it was held
             )doc"))
        .def("addAt", [](Sections& self, const Vars::Section& section, const Sections::Place& place) {
                self.addAt(section.deepcopy(), place);
            }, py::arg("section"), py::arg("place"), py::doc(R"doc(
Adds a copy of a section at a place :meth:`reserve` held, after the sections already added there -- or at the end, when
the variable was cleared since the place was held

Parameters
----------
section: :class:`IfTemplate`
    The section

place: Tuple[:class:`int`, :class:`int`]
    The place
             )doc"))
        .def("names", [](const Sections& self) {
                std::vector<std::string> names;
                for (const Vars::Section* section : self.list()) {
                    names.push_back(section->name);
                }
                return names;
            }, py::doc(R"doc(
The names of the sections, in order

Returns
-------
List[:class:`str`]
    The names
             )doc"))
        .def("empty", &Sections::empty, py::doc(R"doc(
Whether the variable holds no section

Returns
-------
:class:`bool`
    Whether the variable holds no section
             )doc"))
        .def("clear", &Sections::clear, py::doc(R"doc(
Drops every section; a place held before is held no more
             )doc"));

    vars.def(py::init<>())
        .def("sections", &Vars::sections, py::arg("name"), py::return_value_policy::reference_internal, py::doc(R"doc(
The sections variable named 'name', made empty if there is none

Parameters
----------
name: :class:`str`
    The variable's name

Returns
-------
:class:`GraphEditVariables.Sections`
    The variable
            )doc"))
        .def("sectionsNames", &Vars::sectionsNames, py::doc(R"doc(
The names of the sections variables, in the order they were made

Returns
-------
List[:class:`str`]
    The names
            )doc"))
        .def("clear", &Vars::clear, py::doc(R"doc(
Drops every section of every variable; a place held before is held no more
            )doc"));

    auto take = py::class_<AGRC::VarSectionsTake, PyBaseIniGraphGroupEdit, py::smart_holder>(m, "VarSectionsTake", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Copies named `sections`_ of a graph in the first group into a sections variable of a :class:`GraphEditVariables`, and
removes the graph

The usual use: copy some of the mod's own sections into a graph of their own, rename and edit that graph, then take the
result into the variable a :class:`VarSectionsCreate` writes out. The sections taken go after every section the
variable holds when this edit runs -- or, with 'inPlace', where this edit was made: the place is held when the edit is
constructed, so sections added to the variable after that come after them

Parameters
----------
variables: :class:`GraphEditVariables`
    The variables

variable: :class:`str`
    The name of the sections variable to take into

graph: Tuple[:class:`str`, :class:`str`]
    The graph's mod object, in the first group

sections: List[:class:`str`]
    The `sections`_ to take, in order. A name the graph does not hold is skipped

comment: :class:`str`
    A comment written above each section taken :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

inPlace: :class:`bool`
    Whether the sections keep the place held when the edit is made :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc");
    take.def(py::init<Vars&, std::string, AGRC::VarSectionsTake::ModObj, std::vector<std::string>, std::string, bool>(),
             py::arg("variables"), py::arg("variable"), py::arg("graph"), py::arg("sections"), py::arg("comment") = std::string(),
             py::arg("inPlace") = false, py::keep_alive<1, 2>());
    defEdit<AGRC::VarSectionsTake>(take, EditReturns);

    auto create = py::class_<AGRC::VarSectionsCreate, PyBaseIniGraphGroupEdit, py::smart_holder>(m, "VarSectionsCreate", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Writes sections variables of a :class:`GraphEditVariables` out as a graph in each group

For each group, :attr:`variablesOf` names the variables whose `sections`_ make up that group's graph, in order; a
section of a name already in the graph is left out, so the first one written wins. The graph is added under
:attr:`graph` in that group, and a group whose variables hold nothing gets no graph. When there are no groups at all,
the first group's variables are written into a group of its own. Its graph follows ``run =`` by naming the section each
value calls

Parameters
----------
variables: :class:`GraphEditVariables`
    The variables

graph: Tuple[:class:`str`, :class:`str`]
    The mod object the graph is added under in each group

variablesOf: Callable[[:class:`int`], List[:class:`str`]]
    The names of the variables that make up a group's graph, in order, given the group's index
    )doc");
    create.def(py::init([](Vars& variables, AGRC::VarSectionsCreate::ModObj graph, AGRC::VarSectionsCreate::VariablesOf variablesOf) {
               return std::make_unique<AGRC::VarSectionsCreate>(variables, std::move(graph), std::move(variablesOf));
           }), py::arg("variables"), py::arg("graph"), py::arg("variablesOf"), py::keep_alive<1, 2>())
          .def_readwrite("graph", &AGRC::VarSectionsCreate::graph,
                         py::doc(R"doc(Tuple[:class:`str`, :class:`str`]: The mod object the graph is added under in each group)doc"))
          .def_readwrite("variablesOf", &AGRC::VarSectionsCreate::variablesOf, py::doc(R"doc(
Callable[[:class:`int`], List[:class:`str`]]: The names of the variables that make up a group's graph, in order, given
the group's index
          )doc"))
          .def_property("z3", py::cpp_function([](const AGRC::VarSectionsCreate& self) { return self.z3; }, py::return_value_policy::reference),
                        py::cpp_function([](AGRC::VarSectionsCreate& self, AGRC::Z3Context* z3) { self.z3 = z3; }, py::keep_alive<1, 2>()),
                        py::doc(R"doc(
Optional[:class:`Z3Context`]: The context the graph builds its conditions in, or ``None`` for its own
          )doc"));
    defEdit<AGRC::VarSectionsCreate>(create, EditReturns);
}
