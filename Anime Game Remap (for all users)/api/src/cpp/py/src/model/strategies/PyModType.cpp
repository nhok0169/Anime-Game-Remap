#include "PyModType.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppModType(pybind11::module_ &m) {
    // 'Cpp'-prefixed -- unlike 'ModTypeIdData' (which has no pure-Python counterpart of the same
    // exact name), 'ModType' collides with the live pure-Python 'ModType' class in
    // model/strategies/ModType.py, which builds its own richer representation from this C++ data,
    // so the bound name is disambiguated with the 'Cpp' prefix; see Documentation/CLAUDE.md's
    // naming-pitfall section / Architecture/CLAUDE.md's 'Cpp' prefix rule.
    py::class_<AGRC::ModType>(m, "CppModType", R"doc(
Heavy data for a type of mod

Meant to carry the full C++-side representation of a mod type -- contrast with the cheap
:class:`ModTypeIdData` an ini classifier (e.g. :class:`CppBaseIniClassifier`) holds instead. The
Python-side :class:`ModType` is meant to build itself using this data.

Parameters
----------
gameTypeId: :class:`int`
    The id for the game this type of mod belongs to -- stored as-is, with no validation that it
    corresponds to one of :class:`GameTypeId`'s declared values (see :class:`GameTypeIdTools` if
    that's needed)

modTypeId: :class:`int`
    The id for this specific type of mod -- stored as-is, with no validation that it corresponds
    to one of :class:`ModTypeId`'s declared values (see :class:`ModTypeIdTools` if that's needed),
    so a custom mod type using some id not registered in :class:`ModTypeId` can still be represented

name: :class:`str`
    The default name for the type of mod

aliases: Optional[List[:class:`str`]]
    Other alternative names for the type of mod :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``[]``
    )doc")

        .def(py::init<int, int, const std::string &, const std::vector<std::string> &>(), py::arg("gameTypeId"), py::arg("modTypeId"), py::arg("name"), py::arg("aliases") = std::vector<std::string>{})

        .def_readwrite("gameTypeId", &AGRC::ModType::gameTypeId,
    py::doc(R"doc(:class:`int`: The id for the game this type of mod belongs to)doc"))

        .def_readwrite("modTypeId", &AGRC::ModType::modTypeId,
    py::doc(R"doc(:class:`int`: The id for this specific type of mod)doc"))

        .def_readwrite("name", &AGRC::ModType::name,
    py::doc(R"doc(:class:`str`: The default name for the type of mod)doc"))

        .def_readwrite("aliases", &AGRC::ModType::aliases,
    py::doc(R"doc(List[:class:`str`]: Other alternative names for the type of mod)doc"));
}
