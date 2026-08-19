#include "PyModType.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppModType(pybind11::module_ &m) {
    // Kept under the 'Cpp' prefix -- the bare 'ModType' name is already the live, unrelated
    // legacy pure-Python ModType class (model/strategies/ModType.py: hashes/indices/vertexCounts/
    // vgRemaps/ini parse-fix-remove builders and all). This new class isn't a port or replacement
    // of that one -- it's this classifier's own minimal mod-type representation -- so it stays
    // 'Cpp'-prefixed to avoid shadowing a class that's still very much in live use, rather than
    // the usual "shadowing a deprecated ...Old class" reason documented elsewhere in this file set.
    py::class_<AGRC::ModType>(m, "CppModType", R"doc(
Class for defining a type of mod

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
    )doc")

        .def(py::init<int, int>(), py::arg("gameTypeId"), py::arg("modTypeId"))

        .def_readwrite("gameTypeId", &AGRC::ModType::gameTypeId,
    py::doc(R"doc(:class:`int`: The id for the game this type of mod belongs to)doc"))

        .def_readwrite("modTypeId", &AGRC::ModType::modTypeId,
    py::doc(R"doc(:class:`int`: The id for this specific type of mod)doc"));
}
