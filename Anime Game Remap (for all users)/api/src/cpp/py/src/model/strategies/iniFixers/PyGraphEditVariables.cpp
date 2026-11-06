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
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/iniFixers/GraphEditVariables.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphCollectRow.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphVarApply.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphVarTake.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
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

    using Value = AGRC::GraphEditVariables::Value;
    using Rows = std::vector<AGRC::GraphCollectRow<>>;

    // A Python object held as a variable's value: let go of with the GIL held, whichever thread drops the last copy
    struct PyHeld {
        explicit PyHeld(py::object object): object(std::move(object)) {}
        PyHeld(const PyHeld&) = delete;
        PyHeld& operator=(const PyHeld&) = delete;
        ~PyHeld() {
            if (object) {
                py::gil_scoped_acquire gil;
                object = py::object();
            }
        }

        py::object object;
    };

    // A value as Python sees it: the C++ types the binding knows cross as their Python types (a sections variable by
    // reference, kept alive by 'owner' when one is given), a Python object as itself. 'name' names it in an error
    py::object valueToPy(const Value& value, const py::handle& owner, const std::string& name) {
        if (value.empty()) {
            return py::none();
        }
        if (auto* held = value.as<PyHeld>()) {
            return held->object;
        }
        if (auto* sections = value.as<AGRC::GraphEditVariables::Sections>()) {
            return owner ? py::cast(sections, py::return_value_policy::reference_internal, owner)
                         : py::cast(sections, py::return_value_policy::reference);
        }
        if (auto* rows = value.as<Rows>()) {
            return py::cast(*rows);
        }
        if (auto* text = value.as<std::string>()) {
            return py::str(*text);
        }
        if (auto* flag = value.as<bool>()) {
            return py::bool_(*flag);
        }
        if (auto* number = value.as<long long>()) {
            return py::int_(*number);
        }
        if (auto* real = value.as<double>()) {
            return py::float_(*real);
        }
        if (auto* resources = value.as<AGRC::IniParseData::ResourceData>()) {
            return py::cast(*resources);
        }
        if (auto* roles = value.as<AGRC::IniParseData::RoleFileData>()) {
            return py::cast(*roles);
        }
        throw py::type_error("the variable '" + name + "' holds a C++ value Python cannot read");
    }

    // A Python object as a value: the first C++ type the binding knows that takes it, so C++ reads it as that type
    // (a str as a std::string, a list of GraphCollect.Row as rows), or else the object itself
    Value valueFromPy(const py::handle& object) {
        if (py::isinstance<py::bool_>(object)) {
            return Value::of<bool>(object.cast<bool>());
        }
        if (py::isinstance<py::int_>(object)) {
            return Value::of<long long>(object.cast<long long>());
        }
        if (py::isinstance<py::float_>(object)) {
            return Value::of<double>(object.cast<double>());
        }
        if (py::isinstance<py::str>(object)) {
            return Value::of<std::string>(object.cast<std::string>());
        }
        if (py::isinstance<AGRC::IniParseData::ResourceData>(object)) {
            return Value::of<AGRC::IniParseData::ResourceData>(object.cast<AGRC::IniParseData::ResourceData>());
        }
        if (py::isinstance<AGRC::IniParseData::RoleFileData>(object)) {
            return Value::of<AGRC::IniParseData::RoleFileData>(object.cast<AGRC::IniParseData::RoleFileData>());
        }
        if (py::isinstance<py::list>(object) && py::len(object) > 0) {
            bool allRows = true;
            for (auto item : object) {
                allRows = allRows && py::isinstance<AGRC::GraphCollectRow<>>(item);
            }
            if (allRows) {
                return Value::of<Rows>(object.cast<Rows>());
            }
        }
        return Value::make<PyHeld>(py::reinterpret_borrow<py::object>(object));
    }

    // A Python function as an Apply: given the inputs' values as Python sees them (``None`` for a missing one), its
    // result is the new value, ``None`` leaving the variable as it is
    AGRC::GraphEditVariables::Apply applyFromPy(py::object fn) {
        if (fn.is_none()) {
            return nullptr;
        }

        auto held = std::make_shared<PyHeld>(std::move(fn));
        return [held](const std::vector<const Value*>& inputs) -> std::optional<Value> {
            py::gil_scoped_acquire gil;
            py::list values;
            for (const Value* input : inputs) {
                values.append(input == nullptr ? py::none() : valueToPy(*input, py::handle(), "an input"));
            }
            py::object result = held->object(*values);
            if (result.is_none()) {
                return std::nullopt;
            }
            return valueFromPy(result);
        };
    }

    // An Apply as a Python function, for an edit whose function was made in C++
    py::object applyToPy(const AGRC::GraphEditVariables::Apply& fn) {
        if (!fn) {
            return py::none();
        }
        return py::cpp_function([fn](py::args args) -> py::object {
            std::vector<Value> held;
            held.reserve(args.size());
            std::vector<const Value*> inputs;
            for (auto arg : args) {
                held.push_back(arg.is_none() ? Value() : valueFromPy(arg));
                inputs.push_back(arg.is_none() ? nullptr : &held.back());
            }
            std::optional<Value> result = fn(inputs);
            return result.has_value() ? valueToPy(*result, py::handle(), "the result") : py::none();
        });
    }
}


void initCppGraphEditVariables(pybind11::module_ &m) {
    using Vars = AGRC::GraphEditVariables;
    using Sections = Vars::Sections;

    auto vars = py::class_<Vars>(m, "GraphEditVariables", R"doc(
Named variables that edits save data into, and read or write back out later -- the data a list of edits passes from one
edit to another. A variable holds a value of any type, read and written like a dictionary's (``variables[name]``)

A value crosses between Python and C++ as the first type both know: a :class:`str`, :class:`bool`, :class:`int` or
:class:`float`, a list of :class:`GraphCollect.Row` (what a :class:`GraphCollect` files), an
:class:`IniParseData.ResourceData` or :class:`IniParseData.RoleFileData` (each a copy), or a
:class:`GraphEditVariables.Sections`. Any other Python object is held as itself, readable from Python only

A fixer keeps the `sections`_ it writes in one (:class:`GraphEditVariables.Sections`): an edit copies sections of a
graph into it (:class:`GraphVarTake`), code adds sections it builds itself (:meth:`GraphEditVariables.Sections.add`),
and an edit writes it out as a graph (:attr:`GraphCreate.sectionsOf`). A parser's data keeps its variables too
(:meth:`IniParseData.variables`): each :class:`GraphCollect` files its rows into one, and a :class:`GraphVarApply`
makes a variable from others (:meth:`apply`)

.. warning::
    A variable is a snapshot. An edit that changes a graph afterwards does not change what a variable copied from it
    holds, and an edit that changes the graph a variable was written to does not change the variable

:example:

.. code-block:: python

    import FixRaidenBoss2 as FRB

    variables = FRB.GraphEditVariables()
    variables.sections("fix").add(FRB.IfTemplate([FRB.IfContentPart({"handling": [(0, "skip")]}, 0)], name = "TextureOverrideMine"))
    take = FRB.GraphVarTake(variables, "fix", ("", "body"), ["TextureOverrideBody"])
    create = FRB.GraphCreate((0, "", "fixSections"))
    create.sectionsOf = lambda group: variables.sections("fix").list() if group == 0 else []
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
        .def("list", [](py::object self) {
                // each section kept alive by the variable it belongs to
                py::list result;
                for (Vars::Section* section : self.cast<const Sections&>().list()) {
                    result.append(py::cast(section, py::return_value_policy::reference_internal, self));
                }
                return result;
            }, py::doc(R"doc(
The sections, in order -- the variable's own, not copies: what :attr:`GraphCreate.sectionsOf` hands a graph

Returns
-------
List[:class:`IfTemplate`]
    The sections
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
        .def("__getitem__", [](py::object self, const std::string& name) {
                const Value* found = self.cast<const Vars&>().value(name);
                if (found == nullptr) {
                    throw py::key_error("there is no variable '" + name + "'");
                }
                return valueToPy(*found, self, name);
            }, py::arg("name"), py::doc(R"doc(
The value of the variable named 'name' -- a sections variable by reference, valid while the variable is. Raises a
:class:`KeyError` when there is none

Parameters
----------
name: :class:`str`
    The variable's name

Returns
-------
Any
    The value
            )doc"))
        .def("__setitem__", [](Vars& self, const std::string& name, const py::object& value) {
                self.set(name, valueFromPy(value));
            }, py::arg("name"), py::arg("value"), py::doc(R"doc(
Sets the variable named 'name' to 'value', replacing what it held

Parameters
----------
name: :class:`str`
    The variable's name

value: Any
    The value
            )doc"))
        .def("__contains__", &Vars::has, py::arg("name"), py::doc(R"doc(
Whether there is a variable named 'name'

Parameters
----------
name: :class:`str`
    The variable's name

Returns
-------
:class:`bool`
    Whether there is one
            )doc"))
        .def("__delitem__", [](Vars& self, const std::string& name) {
                if (!self.has(name)) {
                    throw py::key_error("there is no variable '" + name + "'");
                }
                self.erase(name);
            }, py::arg("name"), py::doc(R"doc(
Removes the variable named 'name'. Raises a :class:`KeyError` when there is none

Parameters
----------
name: :class:`str`
    The variable's name
            )doc"))
        .def("names", &Vars::names, py::doc(R"doc(
The names of the variables, in the order they were made

Returns
-------
List[:class:`str`]
    The names
            )doc"))
        .def_static("of", [](const std::vector<std::string>& names, const std::vector<py::object>& values) {
                std::vector<Value> held;
                held.reserve(values.size());
                std::vector<const Value*> pointers;
                for (const py::object& value : values) {
                    held.push_back(value.is_none() ? Value() : valueFromPy(value));
                    pointers.push_back(value.is_none() ? nullptr : &held.back());
                }
                return Vars::of(names, pointers);
            }, py::arg("names"), py::arg("values"), py::doc(R"doc(
Variables of the given names holding the given values -- eg. the inputs an :meth:`apply` function is called with, under
their names, for a function that reads its variables by name. A ``None`` value makes no variable

Parameters
----------
names: List[:class:`str`]
    The names

values: List[Any]
    The values, one per name

Returns
-------
:class:`GraphEditVariables`
    The variables
            )doc"))
        .def("apply", [](Vars& self, const std::vector<std::string>& inputs, py::object fn, const std::string& into) {
                self.apply(inputs, applyFromPy(std::move(fn)), into);
            }, py::arg("inputs"), py::arg("fn"), py::arg("into"), py::doc(R"doc(
Makes the variable named 'into' from the variables named 'inputs': 'fn' is called with their values, in order
(``None`` for one that does not exist), and what it returns replaces the value of 'into'. When it returns ``None``,
'into' is left as it is

Parameters
----------
inputs: List[:class:`str`]
    The names of the variables read

fn: Callable[..., Any]
    The function

into: :class:`str`
    The name of the variable written
            )doc"))
        .def("sections", &Vars::sections, py::arg("name"), py::return_value_policy::reference_internal, py::doc(R"doc(
The sections variable named 'name', made empty if there is none. Raises an error when the variable holds a value of
another type

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
Drops every section of every sections variable -- a place held before is held no more -- and every other variable. A
sections variable stays, empty: an edit may hold a place in it
            )doc"));

    auto take = py::class_<AGRC::GraphVarTake, PyBaseIniGraphGroupEdit, py::smart_holder>(m, "GraphVarTake", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Copies named `sections`_ of a graph in the first group into a sections variable of a :class:`GraphEditVariables`, and
removes the graph

The usual use: copy some of the mod's own sections into a graph of their own, rename and edit that graph, then take the
result into the variable a :class:`GraphCreate` writes out (:attr:`GraphCreate.sectionsOf`). The sections taken go after every section the
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
    take.def(py::init<Vars&, std::string, AGRC::GraphVarTake::ModObj, std::vector<std::string>, std::string, bool>(),
             py::arg("variables"), py::arg("variable"), py::arg("graph"), py::arg("sections"), py::arg("comment") = std::string(),
             py::arg("inPlace") = false, py::keep_alive<1, 2>());
    defEdit<AGRC::GraphVarTake>(take, EditReturns);

    auto apply = py::class_<AGRC::GraphVarApply, PyBaseIniGraphGroupEdit, AGRC::IParseResult, py::smart_holder>(m, "GraphVarApply", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit` and :class:`IParseResult`

Makes a variable of the parse from others: when the parser files it, the variables named :attr:`inputs` of the parser's
:meth:`IniParseData.variables` are handed to :attr:`fn`, and what it returns replaces the variable :attr:`into` -- see
:meth:`GraphEditVariables.apply`

Put it in :attr:`GIMIParser.parseSteps` or :attr:`GIMIParser.deferredParseSteps` after the edits that file its inputs
(a :class:`GraphCollect` files its rows in the variable named after it). It changes no graph; it reads only variables,
so anything a graph says is collected into one first

:example:

.. code-block:: python

    import FixRaidenBoss2 as FRB

    resources = FRB.GraphCollect([FRB.IniKeywords.Filename], name = "Resources")
    files = FRB.GraphVarApply(["Resources"], lambda rows: len(rows or []), "ResourceCount")
    parser.deferredParseSteps = [resources, files]
    parser.parseData().variables()["ResourceCount"]

Parameters
----------
inputs: List[:class:`str`]
    The names of the variables read, in the order :attr:`fn` is given them

fn: Callable[..., Any]
    The function making the new value of :attr:`into`: given the inputs' values (``None`` for a missing one), it returns
    the new value, or ``None`` to leave :attr:`into` as it is

into: :class:`str`
    The name of the variable written
    )doc");
    apply.def(py::init([](std::vector<std::string> inputs, py::object fn, std::string into) {
                  return std::make_unique<AGRC::GraphVarApply>(std::move(inputs), applyFromPy(std::move(fn)), std::move(into));
              }), py::arg("inputs"), py::arg("fn"), py::arg("into"))
        .def_readwrite("inputs", &AGRC::GraphVarApply::inputs, R"doc(
The names of the variables read, in the order :attr:`fn` is given them

:type: List[:class:`str`]
        )doc")
        .def_property("fn", [](const AGRC::GraphVarApply& self) { return applyToPy(self.fn); },
                      [](AGRC::GraphVarApply& self, py::object fn) { self.fn = applyFromPy(std::move(fn)); }, R"doc(
The function making the new value of :attr:`into`

:type: Callable[..., Any]
        )doc")
        .def_readwrite("into", &AGRC::GraphVarApply::into, R"doc(
The name of the variable written

:type: :class:`str`
        )doc");
    defEdit<AGRC::GraphVarApply>(apply, EditReturns);
}
