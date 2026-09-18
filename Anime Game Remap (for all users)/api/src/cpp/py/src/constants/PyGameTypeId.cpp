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
        )doc"))

        .def_static("getAliases", &AGRC::GameTypeIdTools::getAliases, py::arg("value"), py::doc(R"doc(
Retrieves the other names a :class:`GameTypeId` also answers to

Mirrors :attr:`ModType.aliases` -- every one of these resolves through :meth:`findByName` exactly as
:meth:`getName`'s answer does

Parameters
----------
value: :class:`GameTypeId`
    The :class:`GameTypeId` to retrieve the aliases for

Returns
-------
List[:class:`str`]
    The aliases for 'value', empty if it has none
        )doc"))

        .def_static("getAll", &AGRC::GameTypeIdTools::getAll, py::doc(R"doc(
Retrieves every :class:`GameTypeId`, in declaration order

The order is stable across runs, so anything listing the supported games (the CLI's ``--help``
epilog) prints them the same way every time

Returns
-------
List[:class:`GameTypeId`]
    All the supported games
        )doc"))

        .def_static("findByName", &AGRC::GameTypeIdTools::findByName, py::arg("name"), py::doc(R"doc(
Finds the :class:`GameTypeId` whose name or alias maximally matches some string -- the
:class:`GameTypeId` counterpart of :meth:`ModTypeIdTools.findByName`

Case and surrounding whitespace are ignored, matching how a mod type's name resolves

.. note::
    Unlike :meth:`ModTypeIdTools.findByName` there is no registry to consult and nothing to
    register: the games are the :class:`GameTypeId` members themselves, so every one of them is
    always findable

Parameters
----------
name: :class:`str`
    The string to search for a game's name/alias within

Returns
-------
Optional[:class:`GameTypeId`]
    The matched :class:`GameTypeId`, if 'name' names one
        )doc"))

        .def_static("getHelpStr", &AGRC::GameTypeIdTools::getHelpStr, py::arg("value"), py::doc(R"doc(
The ``--help`` text describing a single :class:`GameTypeId` -- its name and its aliases

Mirrors :meth:`ModType.getHelpStr`, so the CLI's list of supported games reads exactly like its list
of supported mods

Parameters
----------
value: :class:`GameTypeId`
    The :class:`GameTypeId` to describe

Returns
-------
:class:`str`
    The help text for 'value'
        )doc"));
}
