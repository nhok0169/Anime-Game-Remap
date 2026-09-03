#include "PyIniClassifyStats.h"

#include <tsl/ordered_map.h>

#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/ModTypeIdData.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// tsl::ordered_map has no built-in pybind11 type_caster (unlike the std containers
// <pybind11/stl.h> knows about) -- converted through a py::dict by hand instead, on both the
// constructor and the property. A Python dict already preserves insertion order (3.7+), and
// pybind11's py::dict iterates in that same order, so this round-trips faithfully with
// tsl::ordered_map's own ordering guarantee.
tsl::ordered_map<int, AGRC::ModTypeIdData> modTypeFromDict(const py::dict &d) {
    // emplace, not operator[] -- AGRC::ModTypeIdData has no default constructor (its only
    // constructor takes the required 'gameTypeId'), and operator[] would need one to
    // default-construct a placeholder before overwriting it.
    tsl::ordered_map<int, AGRC::ModTypeIdData> result;
    for (auto item : d) {
        result.emplace(item.first.cast<int>(), item.second.cast<AGRC::ModTypeIdData>());
    }
    return result;
}

py::dict modTypeToDict(const AGRC::IniClassifyStats &self) {
    py::dict d;
    for (const auto &[key, val] : self.modType) {
        d[py::int_(key)] = val;
    }
    return d;
}

}


void initCppIniClassifyStats(pybind11::module_ &m) {
    // Registered under the bare 'IniClassifyStats' name -- the pure-Python original
    // ('IniClassifyStatsOld') has been deleted outright, so this graduates out of the temporary
    // 'Cpp'-prefixed wrapper-outcome-1 naming per Architecture/CLAUDE.md's "Two different outcomes
    // for porting a class" checklist, same as IniFile/GIMIFixer/MultiModFixer/RemapIniRemover
    // before it. No wrapper subclass needed here (unlike when 'modType' was a py::object template
    // param) -- AGRC::IniClassifyStats is a concrete, non-template class now, so it's bound
    // directly.
    py::class_<AGRC::IniClassifyStats>(m, "IniClassifyStats", R"doc(
Stores the statistics about the classification result of a .ini file

Parameters
----------
modType: Dict[:class:`int`, :class:`ModTypeIdData`]
    The types of mod found, keyed by their id

    **Default**: ``{}``

isMod: :class:`bool`
    Whether the .ini file belongs to a mod

    **Default**: ``False``

isFixed: :class:`bool`
    Whether the .ini file is fixed

    **Default**: ``False``
    )doc")

        .def(py::init([](const py::dict &modType, bool isMod, bool isFixed) {
            return AGRC::IniClassifyStats(modTypeFromDict(modType), isMod, isFixed);
        }), py::arg("modType") = py::dict(), py::arg("isMod") = false, py::arg("isFixed") = false)

        .def_property("modType", &modTypeToDict,
            [](AGRC::IniClassifyStats &self, const py::dict &modType) { self.modType = modTypeFromDict(modType); },
    py::doc(R"doc(Dict[:class:`int`, :class:`ModTypeIdData`]: The types of mod found, keyed by their id)doc"))

        .def_readwrite("isMod", &AGRC::IniClassifyStats::isMod,
    py::doc(R"doc(:class:`bool`: Whether the .ini file belongs to a mod)doc"))

        .def_readwrite("isFixed", &AGRC::IniClassifyStats::isFixed,
    py::doc(R"doc(:class:`bool`: Whether the .ini file is fixed)doc"));
}
