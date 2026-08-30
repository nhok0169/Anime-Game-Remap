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
    auto cls = py::class_<PyBaseIniParser>(m, "BaseIniParser", R"doc(
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
