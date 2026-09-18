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

#include "PyBaseIniParser.h"

#include <memory>
#include <utility>

#include "AGRemapCore/model/files/IniFile.h"


PyBaseIniParser::PyBaseIniParser(py::object iniFile):
    PyBaseIniParserCore(nullptr), iniFileObj(std::move(iniFile)), modsToFix(py::set()) {
    // The inherited AGRemapCore::IniFile* used to be left null here, on the grounds that the .ini
    // file a caller hands a parser is the Python one with no C++ counterpart. That stopped being
    // true when the pure-Python IniFile was deleted -- see syncCoreIniFile.
    syncCoreIniFile();
}


void PyBaseIniParser::syncCoreIniFile() {
    // py::isinstance rather than a try/cast, so a Python object that merely quacks like an IniFile
    // still leaves the core pointer null -- the same rule the Py* contexts use.
    AGRC::IniFile *coreIni = nullptr;
    if (!iniFileObj.is_none() && py::isinstance<AGRC::IniFile>(iniFileObj)) {
        coreIni = iniFileObj.cast<AGRC::IniFile*>();
    }

    this->setIniFile(coreIni);
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
