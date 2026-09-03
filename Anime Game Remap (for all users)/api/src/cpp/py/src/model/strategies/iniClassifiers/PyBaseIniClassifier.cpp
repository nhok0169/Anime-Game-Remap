#include "PyBaseIniClassifier.h"

#include <optional>
#include <string>
#include <tuple>
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
    py::class_<AGRC::BaseIniClassifier>(m, "BaseIniClassifier", R"doc(
Base class to help classify the type of mod given the mod's .ini files
    )doc")

        .def(py::init<>())

        // Two overloads mirror the Python original's 'Union[str, List[str]]' 'iniTxt' parameter,
        // same registration-order reasoning as ParseContext's constructor (see PyParseContext.cpp):
        // the str overload is tried first and a Python str can never successfully cast to
        // List[str], so there's no ambiguity the other way either.
        .def("classify", py::overload_cast<const std::string&, std::optional<AGRC::GameTypeId>>(&AGRC::BaseIniClassifier::classify),
    py::arg("iniTxt"), py::arg("gameTypeId") = py::none(), py::doc(R"doc(
Determines the type of mod given the text from the mod's .ini file

Parameters
----------
iniTxt: Union[:class:`str`, List[:class:`str`]]
    The text of the .ini file to read from, given as either:

    * the full text OR
    * lines of text with each line ending with a newline character

gameTypeId: Optional[:class:`GameTypeId`]
    The game the .ini file is expected to belong to, if known

    **Default**: ``None``

Returns
-------
:class:`IniClassifyStats`
    The stats about the classification of the .ini file
        )doc"))

        .def("classify", py::overload_cast<const std::vector<std::string>&, std::optional<AGRC::GameTypeId>>(&AGRC::BaseIniClassifier::classify),
    py::arg("iniTxt"), py::arg("gameTypeId") = py::none())

        // Same str/List[str] overload-order reasoning as classify() above.
        .def("checkIsMod", py::overload_cast<const std::string&, std::optional<AGRC::GameTypeId>>(&AGRC::BaseIniClassifier::checkIsMod),
    py::arg("iniTxt"), py::arg("gameTypeId") = py::none(), py::doc(R"doc(
Determines whether the .ini file belongs to a mod

Cheaper than :meth:`classify` when only this yes/no answer is needed -- see :meth:`classify`'s own
doc comment for what "belongs to a mod" means

Parameters
----------
iniTxt: Union[:class:`str`, List[:class:`str`]]
    The text of the .ini file to read from, given as either:

    * the full text OR
    * lines of text with each line ending with a newline character

gameTypeId: Optional[:class:`GameTypeId`]
    The game the .ini file is expected to belong to, if known

    **Default**: ``None``

Returns
-------
:class:`bool`
    Whether the .ini file belongs to a mod
        )doc"))

        .def("checkIsMod", py::overload_cast<const std::vector<std::string>&, std::optional<AGRC::GameTypeId>>(&AGRC::BaseIniClassifier::checkIsMod),
    py::arg("iniTxt"), py::arg("gameTypeId") = py::none())

        // No overload_cast here -- checkIsFixedMod's bool* out-params have no Python-facing
        // equivalent, so each overload is bound via a lambda that declares real local bools,
        // calls through with their addresses (a plain virtual call on 'self', dispatching
        // correctly to IniClassifier's override the same as classify()/checkIsMod() do), and
        // returns them bundled as a tuple -- same pattern as PyDFA.cpp's 'pytransition' wrapping
        // BaseDFA::transition's own bool*/out-param signature.
        .def("checkIsFixedMod", [](AGRC::BaseIniClassifier &self, const std::string &iniTxt, std::optional<AGRC::GameTypeId> gameTypeId) -> std::tuple<bool, bool> {
            bool isFixed;
            bool isMod;
            self.checkIsFixedMod(iniTxt, &isFixed, &isMod, gameTypeId);
            return {isFixed, isMod};
        }, py::arg("iniTxt"), py::arg("gameTypeId") = py::none(), py::doc(R"doc(
Determines whether the .ini file is fixed and/or belongs to a mod

Cheaper than :meth:`classify` when only these yes/no answers are needed -- see :meth:`classify`'s
own doc comment for what "belongs to a mod"/"is fixed" mean

Parameters
----------
iniTxt: Union[:class:`str`, List[:class:`str`]]
    The text of the .ini file to read from, given as either:

    * the full text OR
    * lines of text with each line ending with a newline character

gameTypeId: Optional[:class:`GameTypeId`]
    The game the .ini file is expected to belong to, if known

    **Default**: ``None``

Returns
-------
Tuple[:class:`bool`, :class:`bool`]
    Whether the .ini file is fixed, and whether it belongs to a mod, in that order
        )doc"))

        .def("checkIsFixedMod", [](AGRC::BaseIniClassifier &self, const std::vector<std::string> &iniTxt, std::optional<AGRC::GameTypeId> gameTypeId) -> std::tuple<bool, bool> {
            bool isFixed;
            bool isMod;
            self.checkIsFixedMod(iniTxt, &isFixed, &isMod, gameTypeId);
            return {isFixed, isMod};
        }, py::arg("iniTxt"), py::arg("gameTypeId") = py::none())

        .def("clear", &AGRC::BaseIniClassifier::clear, py::doc(R"doc(
Clears the state of the classifier
        )doc"));
}
