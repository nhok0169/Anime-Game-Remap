#include "PyIfPredPart.h"

#include <memory>
#include <optional>
#include <string>

#include <pybind11/stl.h>

#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    // Same 'None -> generate a fresh id, otherwise cast straight to size_t' convention as
    // IfTemplatePart/IfContentPart's own constructors (see PyIfContentPart.cpp's parseId).
    std::optional<size_t> parseId(const py::object &id) {
        if (id.is_none()) {
            return std::nullopt;
        }
        return id.cast<size_t>();
    }

    // The fully-qualified dotted path to FixRaidenBoss2.constants.IfPredPartType, derived (once,
    // at module init -- see initCppIfPredPart) from this 'core' module's own '__name__' rather
    // than hardcoded. Same reasoning/pattern as PyBaseTokenizer.cpp's own syntaxErrModulePath():
    // this repo's Unit Tester harness imports the whole package as 'src.py.FixRaidenBoss2'
    // instead of a top-level 'FixRaidenBoss2', so a hardcoded path breaks specifically there.
    std::string &ifPredPartTypeModulePath() {
        static std::string path;
        return path;
    }

    // Imported lazily (only on first real use of toPyType/fromPyType, not at module-load time)
    // for the same circular-import reason as PyBaseTokenizer.cpp's raisePySyntaxErr: 'core' is
    // loaded from inside FixRaidenBoss2/__init__.py's own 'from .core import ...', so importing
    // back up into FixRaidenBoss2 at module-init time here would be a real circular import.
    // Deferring it to first use is safe -- FixRaidenBoss2 has always already finished importing
    // by the time any bound method actually runs.
    py::object pyIfPredPartTypeClass() {
        return py::module_::import(ifPredPartTypeModulePath().c_str()).attr("IfPredPartType");
    }

    // AGRemapCore::IfPredPartType -> the existing pure-Python IfPredPartType enum member.
    // IfPredPart.type deliberately keeps returning that same pure-Python enum (not a new,
    // separately-bound C++ one) -- every *other* real caller across this codebase
    // (IfTemplateTree.py, IniSectionGraph.py, BaseIniFixerOld.py, IniFile.py, ...) already
    // compares '.type' against that pure-Python enum directly, with no reason to touch this
    // class at all, so it has to keep working unchanged.
    py::object toPyType(AGRC::IfPredPartType type) {
        // IfPredPartTypeTools::getName's output ("if"/"else"/"elif"/"endif") is exactly the
        // pure-Python enum's own '.value' set, so constructing the enum by value resolves to the
        // matching member via Python's own Enum machinery -- no manual mapping table needed.
        return pyIfPredPartTypeClass()(AGRC::IfPredPartTypeTools::getName(type));
    }

    // The reverse direction -- a plain 4-way exact match against '.value', deliberately NOT
    // IfPredPartTypeTools::getType (that's a permissive *classifier* over raw predicate text,
    // not an exact enum-value lookup -- a Python IfPredPartType instance's '.value' is always
    // already one of the 4 canonical strings).
    AGRC::IfPredPartType fromPyType(const py::object &pyType) {
        std::string value = pyType.attr("value").cast<std::string>();
        if (value == "if") return AGRC::IfPredPartType::If;
        if (value == "else") return AGRC::IfPredPartType::Else;
        if (value == "elif") return AGRC::IfPredPartType::Elif;
        if (value == "endif") return AGRC::IfPredPartType::EndIf;
        throw py::value_error("Unrecognized IfPredPartType value: '" + value + "'");
    }

    std::unique_ptr<AGRC::IfPredPart> makeIfPredPart(std::string src, const py::object &type, AGRC::Z3Context &z3Ctx,
                                                      AGRC::ParseContext *ctx, std::optional<AGRC::Z3Predicate> query, const py::object &id) {
        return std::make_unique<AGRC::IfPredPart>(std::move(src), fromPyType(type), z3Ctx, ctx, std::move(query), parseId(id));
    }

}


void initCppIfPredPart(pybind11::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    ifPredPartTypeModulePath() = parentPackage.empty() ? "constants.IfPredPartType" : (parentPackage + ".constants.IfPredPartType");

    // Registered with AGRC::IfTemplatePart as its real Python base (already registered by
    // initCppIfContentPart, called before this one in bindings.cpp) so 'isinstance(part,
    // core.IfTemplatePart)' and inherited attribute lookup (e.g. '.id') work for real, matching
    // how IfContentPart itself is registered.
    //
    // Registered under the bare 'IfPredPart' name (no 'Cpp' prefix) -- the deprecated
    // bare-named pure-Python original has since been removed entirely.
    //
    // py::smart_holder (not the default unique_ptr holder) -- required so a unique_ptr<IfPredPart>
    // can be moved *from* Python *into* C++, not just returned to Python (needed by IfTemplate's
    // own constructor/'parts' setter/'add'/'__setitem__', which adopt already-existing Python
    // IfPredPart/IfContentPart objects) -- see IfTemplatePart's own registration (PyIfContentPart.cpp)
    // for the fuller reasoning; the same holder is needed consistently up the inheritance chain.
    py::class_<AGRC::IfPredPart, AGRC::IfTemplatePart, py::smart_holder>(m, "IfPredPart", R"doc(
This class inherits from :class:`IfTemplatePart`

Class for defining the predicate part of an `IfTemplate`, using a `Z3`_ predicate rather than a
`sympy`_ query (see :attr:`query`)

Parameters
----------
src: :class:`str`
    The original string within the `IfTemplate`

type: :class:`IfPredPartType`
    The type of predicate encountered

z3Ctx: :class:`Z3Context`
    The `Z3`_ context :attr:`query` will belong to -- shared across every :class:`IfPredPart`
    constructed against the same :class:`Z3Context`, so the same-named variable across several
    predicates interns to the same `Z3`_ constant

ctx: Optional[:class:`ParseContext`]
    The context for parsing the predicate, if 'type' is :attr:`IfPredPartType.If`/
    :attr:`IfPredPartType.Elif` and 'query' isn't already given :raw-html:`<br />` :raw-html:`<br />`

    If given, this is mutated in place (its ``lines`` replaced with :meth:`getTestStr`'s result)
    so it reflects exactly what was parsed. If ``None``, a fresh, throwaway :class:`ParseContext`
    is constructed internally instead

    **Default**: ``None``

query: Optional[:class:`Z3Predicate`]
    The associated `Z3`_ predicate :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None`` and 'type' is :attr:`IfPredPartType.If`/:attr:`IfPredPartType.Elif`,
    will parse the predicate from 'src' instead (see :meth:`getLogicQuery`)

    **Default**: ``None``

id: Optional[:class:`int`]
    The id for the part. If this parameter is ``None``, will generate a new id for the part.

    **Default**: ``None``
    )doc")

        .def(py::init(&makeIfPredPart), py::arg("src"), py::arg("type"), py::arg("z3Ctx"),
             py::arg("ctx") = py::none(), py::arg("query") = std::nullopt, py::arg("id") = py::none())

        .def_readwrite("src", &AGRC::IfPredPart::src,
    py::doc(R"doc(:class:`str`: The original string within the `IfTemplate`)doc"))

        .def_property("type",
            [](const AGRC::IfPredPart &self) { return toPyType(self.type); },
            [](AGRC::IfPredPart &self, const py::object &type) { self.type = fromPyType(type); },
    py::doc(R"doc(:class:`IfPredPartType`: The type of predicate encountered)doc"))

        .def_readwrite("query", &AGRC::IfPredPart::query,
    py::doc(R"doc(Optional[:class:`Z3Predicate`]: The associated `Z3`_ predicate for this part -- ``None``
for :attr:`IfPredPartType.EndIf`, or when parsing 'src' failed)doc"))

        .def("clone", &AGRC::IfPredPart::clone, py::arg("newId") = false,
    py::doc(R"doc(
Creates a copy of this part

Parameters
----------
newId: :class:`bool`
    Whether to generate a new id for the part

    **Default**: ``False``

Returns
-------
:class:`IfPredPart`
    The cloned part
        )doc"))

        .def("__copy__", [](const AGRC::IfPredPart &self) { return self.clone(false); })
        .def("__deepcopy__", [](const AGRC::IfPredPart &self, const py::dict &) { return self.clone(false); })

        .def("getTestStr", &AGRC::IfPredPart::getTestStr,
    py::doc(R"doc(
Retrieves :attr:`src` with :attr:`type`'s own leading keyword (and, for
:attr:`IfPredPartType.If`/:attr:`IfPredPartType.Elif`, a trailing ``then`` keyword) stripped --
the actual predicate text to parse

Returns
-------
:class:`str`
    The stripped predicate text
        )doc"))

        .def_static("getLogicQuery", &AGRC::IfPredPart::getLogicQuery, py::arg("ctx"), py::arg("z3Ctx"),
    py::doc(R"doc(
Generates the corresponding `Z3`_ predicate from a conditional predicate's source text

Parameters
----------
ctx: :class:`ParseContext`
    The parsing context for reading the conditional predicate

z3Ctx: :class:`Z3Context`
    The `Z3`_ context the generated predicate will belong to

Returns
-------
Optional[:class:`Z3Predicate`]
    The generated `Z3`_ predicate, or ``None`` if 'ctx' could not be tokenized/parsed/converted
        )doc"))

        .def_static("getIfPredStr", &AGRC::IfPredPart::getIfPredStr, py::arg("predicate"),
    py::doc(R"doc(
Generates the .ini predicate text used in the if/else-if/else parts of a .ini file for some
already-built `Z3`_ predicate

Parameters
----------
predicate: :class:`Z3Predicate`
    The predicate to render

Returns
-------
Optional[:class:`str`]
    The generated predicate text, or ``None`` if 'predicate' contains a construct with no .ini
    predicate equivalent
        )doc"))

        .def_static("reparent", &AGRC::IfPredPart::reparent, py::arg("predicate"), py::arg("target"),
    py::doc(R"doc(
Rebuilds 'predicate' as an equivalent :class:`Z3Predicate` belonging to a *different*
:class:`Z3Context` -- the only way to move a predicate across `Z3`_ contexts at all, since two
predicates can only be combined (eg. via ``&``) when they already share the same context (see
:class:`Z3Predicate`'s own warning)

Parameters
----------
predicate: :class:`Z3Predicate`
    The predicate to reparent

target: :class:`Z3Context`
    The `Z3`_ context the returned predicate will belong to

Returns
-------
Optional[:class:`Z3Predicate`]
    The reparented predicate, or ``None`` if 'predicate' contains a construct with no .ini
    predicate equivalent, or otherwise fails to re-parse against 'target'
        )doc"))

        .def("toStr", &AGRC::IfPredPart::toStr, py::arg("linePrefix") = std::nullopt,
    py::doc(R"doc(
Retrieves the part as a string

Parameters
----------
linePrefix: Optional[:class:`str`]
    The string that will prefix :attr:`src` :raw-html:`<br />` :raw-html:`<br />`

    If ``None``, :attr:`src` is used as-is. Otherwise, any left spacing from :attr:`src` is
    stripped and 'linePrefix' is prepended instead

    **Default**: ``None``

Returns
-------
:class:`str`
    The string representation of the part
        )doc"));
}
