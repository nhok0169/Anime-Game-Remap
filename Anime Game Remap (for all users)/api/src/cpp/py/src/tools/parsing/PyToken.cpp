#include "PyToken.h"

#include <optional>
#include <string>

#include <pybind11/stl.h>

#include "AGRemapCore/tools/parsing/Token.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppToken(pybind11::module_ &m) {
    // Registered under the bare 'Token' name (no 'Cpp' prefix) -- the deprecated bare-named
    // pure-Python class is now 'TokenOld', so nothing shadows this registration; see
    // Documentation/CLAUDE.md's naming-pitfall section.
    py::class_<AGRC::Token>(m, "Token", R"doc(
A token when parsing some language

Parameters
----------
type: Optional[:class:`str`]
    The name for the type of token, if available

val: :class:`str`
    The value of the token

lineNo: :class:`int`
    The line number the token belongs to

charNo: :class:`int`
    The character number the token belongs to within some line
    )doc")

        .def(py::init<std::optional<std::string>, std::string, size_t, size_t>(),
    py::arg("type"), py::arg("val"), py::arg("lineNo"), py::arg("charNo"))

        .def_readwrite("type", &AGRC::Token::type,
    py::doc(R"doc(Optional[:class:`str`]: The name for the type of token, if available)doc"))

        .def_readwrite("val", &AGRC::Token::val,
    py::doc(R"doc(:class:`str`: The value of the token)doc"))

        .def_readwrite("lineNo", &AGRC::Token::lineNo,
    py::doc(R"doc(:class:`int`: The line number the token belongs to)doc"))

        .def_readwrite("charNo", &AGRC::Token::charNo,
    py::doc(R"doc(:class:`int`: The character number the token belongs to within some line)doc"));
}
