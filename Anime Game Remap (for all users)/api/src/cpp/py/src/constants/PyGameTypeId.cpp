#include "PyGameTypeId.h"

#include <optional>

#include <pybind11/stl.h>

#include "AGRemapCore/constants/GameTypeId.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppGameTypeId(pybind11::module_ &m) {
    // Registered under the bare 'GameTypeId' name (no 'Cpp' prefix) -- no pure-Python class of this
    // exact bare name exists to shadow (the pure-Python equivalent is 'GameTypeNames', a
    // differently-named Enum in constants/GameTypeNames.py), so nothing to disambiguate from; see
    // Documentation/CLAUDE.md's naming-pitfall section / Architecture/CLAUDE.md's 'Cpp' prefix rule.
    py::enum_<AGRC::GameTypeId>(m, "GameTypeId", R"doc(
The names of the different supported games
    )doc")
        .value("GI", AGRC::GameTypeId::GI, R"doc(Genshin Impact)doc")
        .value("WuWa", AGRC::GameTypeId::WuWa, R"doc(Wuthering Waves)doc");

    // Also bare-named -- no pure-Python 'GameTypeIdTools' class exists to shadow either.
    py::class_<AGRC::GameTypeIdTools>(m, "GameTypeIdTools", R"doc(
Tools for handling :class:`GameTypeId`
    )doc")
        .def_static("getEnum", &AGRC::GameTypeIdTools::getEnum, py::arg("value"), py::doc(R"doc(
Retrieves the corresponding :class:`GameTypeId` for some integer value, checking that the value
actually corresponds to one of :class:`GameTypeId`'s declared values

Parameters
----------
value: :class:`int`
    The integer value to convert

Returns
-------
Optional[:class:`GameTypeId`]
    The corresponding :class:`GameTypeId`, if 'value' is valid
        )doc"))

        .def_static("getName", &AGRC::GameTypeIdTools::getName, py::arg("value"), py::doc(R"doc(
Retrieves the corresponding name for a :class:`GameTypeId`

Parameters
----------
value: :class:`GameTypeId`
    The :class:`GameTypeId` to retrieve the name for

Returns
-------
:class:`str`
    The name for 'value'
        )doc"));
}
