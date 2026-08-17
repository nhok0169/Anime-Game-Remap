#include "PyParseContext.h"

#include <optional>
#include <string>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/tools/parsing/ParseContext.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppParseContext(pybind11::module_ &m) {
    // Registered under the bare 'ParseContext' name (no 'Cpp' prefix) -- the deprecated
    // bare-named pure-Python class is now 'ParseContextOld', so nothing shadows this
    // registration; see Documentation/CLAUDE.md's naming-pitfall section.
    //
    // Two constructor overloads mirror the Python class's 'Union[str, List[str]]' 'src'
    // parameter -- pybind11 tries registered overloads in registration order, so the
    // std::string overload (registered first) claims an actual Python str before the
    // std::vector<std::string> overload gets a chance; a std::string can never successfully
    // cast from a Python list, so there's no ambiguity the other way either.
    py::class_<AGRC::ParseContext>(m, "ParseContext", R"doc(
Context for parsing some text

Parameters
----------
src: Union[:class:`str`, List[:class:`str`]]
    The source text to parse

    If this argument is a list, then assumes that the lines of the source text is given

    **Default**: ``""``

file: Optional[:class:`str`]
    The file path to the source text

    **Default**: ``None``

startLineNo: :class:`int`
    The starting line of the source text

    **Default**: ``1``
    )doc")

        .def(py::init<std::string, std::optional<std::string>, size_t>(),
    py::arg("src") = std::string(""), py::arg("file") = std::nullopt, py::arg("startLineNo") = 1)

        .def(py::init<std::vector<std::string>, std::optional<std::string>, size_t>(),
    py::arg("src"), py::arg("file") = std::nullopt, py::arg("startLineNo") = 1)

        .def_readwrite("lines", &AGRC::ParseContext::lines,
    py::doc(R"doc(List[:class:`str`]: The lines of the source text)doc"))

        .def_readwrite("file", &AGRC::ParseContext::file,
    py::doc(R"doc(Optional[:class:`str`]: The file path to the source text)doc"))

        .def_readwrite("startLineNo", &AGRC::ParseContext::startLineNo,
    py::doc(R"doc(:class:`int`: The starting line of the source text)doc"))

        .def("getEndLineNo", &AGRC::ParseContext::getEndLineNo, py::doc(R"doc(
Retrieves the line number after the last line

Returns
-------
:class:`int`
    The ending line number after the last line
        )doc"));
}
