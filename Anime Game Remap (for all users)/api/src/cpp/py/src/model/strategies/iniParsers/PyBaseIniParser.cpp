#include "PyBaseIniParser.h"

#include <memory>
#include <utility>


PyBaseIniParser::PyBaseIniParser(py::object iniFile):
    PyBaseIniParserCore(nullptr), iniFileObj(std::move(iniFile)), modsToFix(py::set()) {
    // The inherited AGRemapCore::IniFile* is always nullptr: the .ini file a real caller hands a
    // parser is the Python one, which has no C++ counterpart to point at. See
    // AGRemapCore::IniParseContext's own note.
}


py::object PyBaseIniParser::parseToPy() {
    parse();
    return py::list();
}


void PyBaseIniParser::clear() {
    PyBaseIniParserCore::clear();

    if (!modsToFix.is_none()) {
        modsToFix.attr("clear")();
    }
}


void initCppBaseIniParser(pybind11::module_ &m) {
    py::class_<PyBaseIniParserCore, py::smart_holder>(m, "CppBaseIniParser", R"doc(
The shared C++ base of every parser, exposed so that one built on the C++ side -- by a
:class:`IniParseBuilder`'s default factory, or by anything in ``AGRemapCore`` -- can still cross into
`Python`_ :raw-html:`<br />` :raw-html:`<br />`

Not usually what you want: a parser created **from** `Python`_ is a :class:`BaseIniParser`, which
inherits from this and carries the extra `Python`_ state. This class exists so the boundary never
has to hand back ``None`` for a core-side object it has no richer type for
    )doc");


    auto cls = py::class_<PyBaseIniParser, PyBaseIniParserCore, py::smart_holder>(m, "BaseIniParser", R"doc(
Base class to parse a .ini file

Parameters
----------
iniFile: :class:`IniFile`
    The .ini file to parse
    )doc");

    cls.def(py::init([](py::object iniFile) {
        return std::make_unique<PyBaseIniParser>(std::move(iniFile));
    }), py::arg("iniFile") = py::none());

    bindBaseIniParserCommonMethods<PyBaseIniParser>(cls, R"doc(
Parses the .ini file

Returns
-------
List[:class:`IniGraphGroup`]
    The parsed groups of caller/callee graphs found in the .ini file -- always empty here, since
    this base class parses nothing. See :meth:`GIMIParser.parse` for a real one
    )doc");
}
