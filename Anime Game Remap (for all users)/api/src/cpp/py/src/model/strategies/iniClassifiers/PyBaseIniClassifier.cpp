#include "PyBaseIniClassifier.h"

#include <optional>
#include <string>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/constants/GameTypeId.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBaseIniClassifier(pybind11::module_ &m) {
    // No trampoline: classify() is expected to be overridden by future C++ subclasses (e.g.
    // IniClassifier), not from pure Python -- see Architecture/CLAUDE.md's note on when a
    // trampoline is/isn't actually needed. AGRC::BaseIniClassifier is bound directly (no wrapper
    // subclass) now that it's a concrete, non-template class -- classify()'s return type
    // (AGRC::IniClassifyStats) is itself directly registered, so there's no more base-type-in-
    // return-position gotcha to route around with a lambda, unlike the earlier py::object-template
    // version of this file.
    py::class_<AGRC::BaseIniClassifier>(m, "CppBaseIniClassifier", R"doc(
Base class to help classify the type of mod given the mod's .ini files
    )doc")

        .def(py::init<>())

        // Two overloads mirror the Python original's 'Union[str, List[str]]' 'iniTxt' parameter,
        // same registration-order reasoning as ParseContext's constructor (see PyParseContext.cpp):
        // the str overload is tried first and a Python str can never successfully cast to
        // List[str], so there's no ambiguity the other way either.
        .def("classify", py::overload_cast<const std::string&, bool, bool, std::optional<AGRC::GameTypeId>>(&AGRC::BaseIniClassifier::classify),
    py::arg("iniTxt"), py::arg("checkIsMod") = true, py::arg("checkIsFixed") = true, py::arg("gameTypeId") = py::none(), py::doc(R"doc(
Determines the type of mod given the text from the mod's .ini file

Parameters
----------
iniTxt: Union[:class:`str`, List[:class:`str`]]
    The text of the .ini file to read from, given as either:

    * the full text OR
    * lines of text with each line ending with a newline character

checkIsMod: :class:`bool`
    Whether to fully check the .ini file belongs to a mod

    **Default**: ``True``

checkIsFixed: :class:`bool`
    Whether to fully check the .ini file has been fixed

    **Default**: ``True``

gameTypeId: Optional[:class:`GameTypeId`]
    The game the .ini file is expected to belong to, if known

    **Default**: ``None``

Returns
-------
:class:`CppIniClassifyStats`
    The stats about the classification of the .ini file
        )doc"))

        .def("classify", py::overload_cast<const std::vector<std::string>&, bool, bool, std::optional<AGRC::GameTypeId>>(&AGRC::BaseIniClassifier::classify),
    py::arg("iniTxt"), py::arg("checkIsMod") = true, py::arg("checkIsFixed") = true, py::arg("gameTypeId") = py::none())

        .def("clear", &AGRC::BaseIniClassifier::clear, py::doc(R"doc(
Clears the state of the classifier
        )doc"));
}
