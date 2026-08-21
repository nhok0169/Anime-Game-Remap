#include "PyModTypeIdData.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppModTypeIdData(pybind11::module_ &m) {
    // Bare-named (no 'Cpp' prefix) -- unlike the earlier 'ModType' name this class used to have,
    // 'ModTypeIdData' has no pure-Python class of the same exact name to shadow (the live, unrelated
    // legacy pure-Python class is 'ModType', in model/strategies/ModType.py -- see that class's own
    // note on building its own richer representation from this cheap data), so nothing to
    // disambiguate from; see Documentation/CLAUDE.md's naming-pitfall section /
    // Architecture/CLAUDE.md's 'Cpp' prefix rule.
    py::class_<AGRC::ModTypeIdData>(m, "ModTypeIdData", R"doc(
Cheap data for a type of mod, held by an ini classifier (e.g. :class:`CppBaseIniClassifier`)

Not meant to be a full representation of a mod type on its own -- the Python-side :class:`ModType`
is meant to build its own richer representation from this data

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

        .def_readwrite("gameTypeId", &AGRC::ModTypeIdData::gameTypeId,
    py::doc(R"doc(:class:`int`: The id for the game this type of mod belongs to)doc"))

        .def_readwrite("modTypeId", &AGRC::ModTypeIdData::modTypeId,
    py::doc(R"doc(:class:`int`: The id for this specific type of mod)doc"));
}
