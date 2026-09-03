#include "PyModType.h"

#include <optional>

#include "../files/PyIniFile.h"
#include "iniFixers/PyIniFixBuilder.h"
#include "iniParsers/PyIniParseBuilder.h"
#include "iniRemovers/PyIniRemoveBuilder.h"
#include "../../tools/PyRanges.h"

#include "AGRemapCore/model/files/IniFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {
    // Carried between initCppModType and initCppModTypeLateBindings -- see PyModType.h for why
    // fixIni cannot be registered in the first pass.
    std::optional<py::class_<AGRC::ModType>> &modTypeClsSlot() {
        static std::optional<py::class_<AGRC::ModType>> slot;
        return slot;
    }
}


// Only ever called after initCppModType has filled the slot -- bindings.cpp registers the two in
// that order, and the late pass has nothing to attach to otherwise.
static py::class_<AGRC::ModType> &modTypeCls() {
    return *modTypeClsSlot();
}


void initCppModType(pybind11::module_ &m) {
    // 'Cpp'-prefixed -- unlike 'ModTypeIdData' (which has no pure-Python counterpart of the same
    // exact name), 'ModType' collides with the live pure-Python 'ModType' class in
    // model/strategies/ModType.py, which builds its own richer representation from this C++ data,
    // so the bound name is disambiguated with the 'Cpp' prefix; see Documentation/CLAUDE.md's
    // naming-pitfall section / Architecture/CLAUDE.md's 'Cpp' prefix rule.
    modTypeClsSlot().emplace(m, "CppModType", R"doc(
Heavy data for a type of mod

Meant to carry the full C++-side representation of a mod type -- contrast with the cheap
:class:`ModTypeIdData` an ini classifier (e.g. :class:`BaseIniClassifier`) holds instead. The
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
    )doc");

    modTypeCls()

        .def(py::init<int, int, const std::string &, const std::vector<std::string> &>(), py::arg("gameTypeId"), py::arg("modTypeId"), py::arg("name"), py::arg("aliases") = std::vector<std::string>{})

        .def_readwrite("gameTypeId", &AGRC::ModType::gameTypeId,
    py::doc(R"doc(:class:`int`: The id for the game this type of mod belongs to)doc"))

        .def_readwrite("modTypeId", &AGRC::ModType::modTypeId,
    py::doc(R"doc(:class:`int`: The id for this specific type of mod)doc"))

        .def_readwrite("name", &AGRC::ModType::name,
    py::doc(R"doc(:class:`str`: The default name for the type of mod)doc"))

        .def_readwrite("aliases", &AGRC::ModType::aliases,
    py::doc(R"doc(List[:class:`str`]: Other alternative names for the type of mod)doc"))

        .def("isName", &AGRC::ModType::isName, py::arg("name"), py::doc(R"doc(
Determines whether this mod type goes by some name

Compared case-insensitively against :attr:`CppModType.name` and every entry in
:attr:`CppModType.aliases`

Parameters
----------
name: :class:`str`
    The name to check

Returns
-------
:class:`bool`
    Whether this mod type goes by 'name'
        )doc"))

        .def("getModsToFix", &AGRC::ModType::getModsToFix, py::doc(R"doc(
Retrieves the names of the mods this mod type can be fixed onto

.. warning::
    **Deliberately not bug-compatible with the pure-Python** :meth:`ModType.getModsToFix`. That one
    unions ``hashes.fixTo`` and ``indices.fixTo`` -- two sets it declares and then never populates
    anywhere, so it returns an empty set for every mod type, always. This reads the remap targets
    that actually exist

Returns
-------
Set[:class:`str`]
    The names of the mods to fix to
        )doc"))

        .def("getVertexCount", &AGRC::ModType::getVertexCount, py::arg("version") = py::none(), py::doc(R"doc(
Retrieves the number of vertices for this mod

Parameters
----------
version: Optional[:class:`CppVersion`]
    The game version wanted :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest

Returns
-------
Optional[:class:`int`]
    The vertex count, or ``None`` if this mod type has no row for it
        )doc"))

        .def("getVGRemap", &AGRC::ModType::getVGRemap, py::arg("modName"), py::arg("fromVersion") = py::none(),
             py::arg("toVersion") = py::none(), py::arg("fromComp") = py::none(), py::arg("toComp") = py::none(),
             py::doc(R"doc(
Retrieves the vertex group remap for fixing this mod type onto another

Parameters
----------
modName: :class:`str`
    The name of the mod being fixed onto

fromVersion: Optional[:class:`CppVersion`]
    The version being fixed from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest

toVersion: Optional[:class:`CppVersion`]
    The version being fixed to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest

fromComp: Optional[:class:`str`]
    The component being fixed from. ``None`` leaves the column unconstrained :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``None``

toComp: Optional[:class:`str`]
    The component being fixed onto, with the same ``None`` meaning :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
Optional[:class:`VGRemap`]
    The remap, or ``None`` if the table has no matching row
        )doc"))

        .def("getHelpStr", &AGRC::ModType::getHelpStr, py::doc(R"doc(
Retrieves the help text describing this mod type, as the CLI prints it

Returns
-------
:class:`str`
    The help text
        )doc"))

        .def("getHashRanges", [](const AGRC::ModType &self, const AGRC::IfContentPartColouring<std::string, std::string> &partColours,
                                 const std::optional<AGRC::Version> &version, const py::object &nonVersionVals) {
            // Built fresh per call from py::none() rather than defaulted to a container literal --
            // py::arg("x") = <mutable container> is pybind11's version of Python's
            // mutable-default-argument bug, and every caller would share the one instance.
            std::vector<std::optional<std::string>> parsed;
            if (!nonVersionVals.is_none()) {
                parsed = nonVersionVals.cast<std::vector<std::optional<std::string>>>();
            }

            // The bound "Ranges" class is the PyRanges<long long> wrapper, not AGRC::Ranges
            // itself -- returning the core type raises "Unregistered type" at call time.
            AGRC::Ranges<long long> result = self.getHashRanges(partColours, version, parsed);
            return PyRanges<long long>(result.ranges, false);
        }, py::arg("partColours"), py::arg("version") = py::none(), py::arg("nonVersionVals") = py::none(),
             py::doc(R"doc(
Retrieves the valid ranges of order indices within an :class:`IfContentPart` whose ``hash`` values
belong to this mod type

Parameters
----------
partColours: :class:`IfContentPartColouring`
    The current states of the :class:`IfContentPart`

version: Optional[:class:`CppVersion`]
    The version the hashes should come from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning any

nonVersionVals: Optional[List[Optional[:class:`str`]]]
    Values for the non-version index columns, used to narrow which instance of a hash is wanted
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`Ranges`
    The valid ranges of indices
        )doc"));
}


void initCppModTypeLateBindings(pybind11::module_ &m) {
    // The three builders are registered after this file's first pass (their build() signatures
    // name IniFile), and pybind11 bakes an attribute's type string at registration time -- so
    // these have to wait here or core.pyi gets a raw "AGRemapCore::IniFixBuilder".
    // Read/write so a caller can swap in their own, exactly as the pure-Python ModType allows.
    modTypeCls()
        .def_readwrite("iniParseBuilder", &AGRC::ModType::iniParseBuilder,
    py::doc(R"doc(:class:`CppIniParseBuilder`: The builder for the parser that reads a .ini file of this mod type)doc"))

        .def_readwrite("iniFixBuilder", &AGRC::ModType::iniFixBuilder,
    py::doc(R"doc(:class:`CppIniFixBuilder`: The builder for the fixer that fixes a .ini file of this mod type)doc"))

        .def_readwrite("iniRemoveBuilder", &AGRC::ModType::iniRemoveBuilder,
    py::doc(R"doc(:class:`CppIniRemoveBuilder`: The builder for the remover that removes a previous fix)doc"));

    // See PyModType.h for why this is a second pass rather than part of the chain above.
    // See PyModType.h for why this is a second pass rather than part of the chain above.
    modTypeCls().def("fixIni", &AGRC::ModType::fixIni, py::arg("iniFile"), py::arg("keepBackup") = true,
                     py::arg("fixOnly") = false, py::doc(R"doc(
Fixes a .ini file, but **only if that file was classified as this mod type** -- a no-op otherwise

Returns nothing, matching the pure-Python original: the fix it produces is written out by
:meth:`IniFile.fix` rather than handed back. Call that directly to see it

Parameters
----------
iniFile: :class:`IniFile`
    The .ini file to fix

keepBackup: :class:`bool`
    Whether to keep a backup copy of the original .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

fixOnly: :class:`bool`
    Whether to only fix without removing any previous fix :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
        )doc"));
}
