#include "PyRemapService.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>

#include <pybind11/stl.h>

#include <tsl/ordered_set.h>

#include "AGRemapCore/RemapService.h"
#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/view/BaseLogger.h"
#include "model/PyVersion.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// The same value-based crossing PyIniFile does for this enum, and for the same reason: DownloadMode
// is a plain C++ enum here but a StrEnum on the Python side, so it travels by value rather than by
// identity. Duplicated rather than shared because the two files are the only users and a shared
// header for two switch statements would be more indirection than it saves.
const char* downloadModeName(AGRC::DownloadMode mode) {
    switch (mode) {
        case AGRC::DownloadMode::Disabled: return "disabled";
        case AGRC::DownloadMode::Always: return "always";
        default: return "normal";
    }
}


AGRC::DownloadMode parseDownloadMode(const py::object &mode) {
    if (mode.is_none()) {
        return AGRC::DownloadMode::Normal;
    }

    py::object value = py::hasattr(mode, "value") ? mode.attr("value") : mode;
    std::string parsed = py::str(value).cast<std::string>();

    if (parsed == "disabled") {
        return AGRC::DownloadMode::Disabled;
    }

    if (parsed == "always") {
        return AGRC::DownloadMode::Always;
    }

    if (parsed == "normal") {
        return AGRC::DownloadMode::Normal;
    }

    throw py::value_error("Unknown download mode: '" + parsed + "'");
}


// tsl::ordered_set has no pybind type_caster, and the order is meaningful, so it crosses as a LIST
// -- the same treatment IniFile::defaultModTypeIds gets. See PyIniFile.cpp.
tsl::ordered_set<int> toOrderedSet(const py::object &value) {
    tsl::ordered_set<int> result;
    if (value.is_none()) {
        return result;
    }

    for (const py::handle &item : value) {
        result.insert(item.cast<int>());
    }

    return result;
}

}


void initCppRemapService(py::module_ &m) {
    py::class_<AGRC::RemapService>(m, "RemapService", R"doc(
The overall class for remapping mods -- the **model** half, with no UI of its own

:raw-html:`<br />`

Everything on the other side of that line -- turning a user's mod-type *names*, version *strings*
and download-mode *strings* into the ids/versions/enums this class takes, writing the log file,
printing the tips -- belongs to :class:`RemapServiceCLI`, which wraps one of these

.. note::
    Every mod-type argument here is a **set of ids**, following :class:`IniFile`'s own convention
    exactly: ``None`` means *no filter at all*, while an empty set means *accept nothing*. Those are
    two different answers, which is why they are ``Optional`` rather than plain sets

Parameters
----------
path: Optional[:class:`str`]
    Where to run the fix from. ``None`` runs it from wherever the software started

keepBackups: :class:`bool`
    Whether to keep backup versions of any .ini files the fix changes

fixOnly: :class:`bool`
    Whether to only fix, without removing previous fixes

undoOnly: :class:`bool`
    Whether to only undo previous fixes

hideOrig: :class:`bool`
    Whether to not show the mod on the original character

readAllInis: :class:`bool`
    Whether to read every .ini file encountered

fromModTypeIds: Optional[Set[:class:`int`]]
    The :class:`ModTypeId` values to accept when parsing

forcedModTypeIds: Optional[Set[:class:`int`]]
    The :class:`ModTypeId` values to forcibly assume for the parsed .ini files

defaultModTypeIds: Optional[List[:class:`int`]]
    The :class:`ModTypeId` values to fall back on when the classifier recognises nothing. Ordered,
    so it crosses as a list rather than a set

handleExceptions: :class:`bool`
    Whether to stop the fix quietly when an exception is caught, rather than raising

fromVersion: Optional[:class:`CppVersion`]
    The game version the parsed .ini files originate from

toModTypeIds: Optional[Set[:class:`int`]]
    The :class:`ModTypeId` values to accept when fixing

proxy: Optional[:class:`str`]
    The proxy server used for internet requests

downloadMode: Optional[:class:`DownloadMode`]
    How file downloads are handled. ``None`` means :attr:`DownloadMode.Normal`

gameTypeId: Optional[:class:`int`]
    The :class:`GameTypeId` value of the game being remapped

logger: Optional[:class:`BaseLogger`]
    Where the fix reports progress. ``None`` means nowhere
    )doc")

        .def(py::init([](std::optional<std::string> path, bool keepBackups, bool fixOnly, bool undoOnly,
                         bool hideOrig, bool readAllInis,
                         std::optional<std::unordered_set<int>> fromModTypeIds,
                         std::optional<std::unordered_set<int>> forcedModTypeIds,
                         const py::object &defaultModTypeIds, bool handleExceptions,
                         std::optional<AGRC::Version> fromVersion,
                         std::optional<std::unordered_set<int>> toModTypeIds,
                         std::optional<std::string> proxy, const py::object &downloadMode,
                         std::optional<int> gameTypeId, std::shared_ptr<AGRC::BaseLogger> logger) {
            return std::make_unique<AGRC::RemapService>(
                std::move(path), keepBackups, fixOnly, undoOnly, hideOrig, readAllInis,
                std::move(fromModTypeIds), std::move(forcedModTypeIds), toOrderedSet(defaultModTypeIds),
                handleExceptions, std::move(fromVersion), std::move(toModTypeIds), std::move(proxy),
                parseDownloadMode(downloadMode), gameTypeId, std::move(logger));
        }), py::arg("path") = py::none(), py::arg("keepBackups") = true, py::arg("fixOnly") = false,
            py::arg("undoOnly") = false, py::arg("hideOrig") = false, py::arg("readAllInis") = false,
            py::arg("fromModTypeIds") = py::none(), py::arg("forcedModTypeIds") = py::none(),
            py::arg("defaultModTypeIds") = py::none(), py::arg("handleExceptions") = false,
            py::arg("fromVersion") = py::none(), py::arg("toModTypeIds") = py::none(),
            py::arg("proxy") = py::none(), py::arg("downloadMode") = py::none(),
            py::arg("gameTypeId") = py::none(), py::arg("logger") = nullptr)

        .def_readwrite("keepBackups", &AGRC::RemapService::keepBackups,
    py::doc(R"doc(:class:`bool`: Whether to keep backup versions of any .ini files the fix changes)doc"))

        .def_readwrite("fixOnly", &AGRC::RemapService::fixOnly,
    py::doc(R"doc(:class:`bool`: Whether to only fix, without removing previous fixes)doc"))

        .def_readwrite("undoOnly", &AGRC::RemapService::undoOnly,
    py::doc(R"doc(:class:`bool`: Whether to only undo previous fixes)doc"))

        .def_readwrite("hideOrig", &AGRC::RemapService::hideOrig,
    py::doc(R"doc(:class:`bool`: Whether to not show the mod on the original character)doc"))

        .def_readwrite("readAllInis", &AGRC::RemapService::readAllInis,
    py::doc(R"doc(:class:`bool`: Whether to read every .ini file encountered)doc"))

        .def_readwrite("fromModTypeIds", &AGRC::RemapService::fromModTypeIds,
    py::doc(R"doc(Optional[Set[:class:`int`]]: The mod types to accept when parsing

``None`` means no filter; an empty set means accept nothing)doc"))

        .def_readwrite("forcedModTypeIds", &AGRC::RemapService::forcedModTypeIds,
    py::doc(R"doc(Optional[Set[:class:`int`]]: The mod types to forcibly assume for the parsed .ini files)doc"))

        .def_property("defaultModTypeIds",
            [](const AGRC::RemapService &self) {
                py::list result;
                for (int modTypeId : self.defaultModTypeIds) {
                    result.append(py::int_(modTypeId));
                }
                return result;
            },
            [](AGRC::RemapService &self, const py::object &value) {
                self.defaultModTypeIds = toOrderedSet(value);
            },
    py::doc(R"doc(List[:class:`int`]: The mod types to fall back on when the classifier recognises nothing

Reads back as a **list**, not a set: the order is the order they land in :meth:`IniFile.getModTypes`)doc"))

        .def_readwrite("handleExceptions", &AGRC::RemapService::handleExceptions,
    py::doc(R"doc(:class:`bool`: Whether to stop the fix quietly when an exception is caught)doc"))

        .def_readwrite("fromVersion", &AGRC::RemapService::fromVersion,
    py::doc(R"doc(Optional[:class:`CppVersion`]: The game version the parsed .ini files originate from)doc"))

        .def_readwrite("toModTypeIds", &AGRC::RemapService::toModTypeIds,
    py::doc(R"doc(Optional[Set[:class:`int`]]: The mod types to accept when fixing)doc"))

        .def_readwrite("proxy", &AGRC::RemapService::proxy,
    py::doc(R"doc(Optional[:class:`str`]: The proxy server used for internet requests)doc"))

        .def_property("downloadMode",
            [](const AGRC::RemapService &self) { return py::str(downloadModeName(self.downloadMode)); },
            [](AGRC::RemapService &self, const py::object &value) { self.downloadMode = parseDownloadMode(value); },
    py::doc(R"doc(:class:`DownloadMode`: How file downloads are handled

Reads back as the :class:`DownloadMode` string value; accepts either a :class:`DownloadMode` or its
value when set)doc"))

        .def_readwrite("gameTypeId", &AGRC::RemapService::gameTypeId,
    py::doc(R"doc(Optional[:class:`int`]: The :class:`GameTypeId` value of the game being remapped)doc"))

        .def_readwrite("logger", &AGRC::RemapService::logger,
    py::doc(R"doc(Optional[:class:`BaseLogger`]: Where the fix reports progress

``None`` means nowhere -- messages are dropped rather than buffered)doc"))

        .def_property_readonly("path", &AGRC::RemapService::path,
    py::doc(R"doc(:class:`str`: The file path the fix runs from)doc"))

        .def("setPath", &AGRC::RemapService::setPath, py::arg("newPath"), py::doc(R"doc(
Sets where the fix runs from, clearing any statistics gathered for the previous path

Parameters
----------
newPath: Optional[:class:`str`]
    The new path, or ``None`` to run from wherever the software started
        )doc"))

        .def_property_readonly("pathIsCwd", &AGRC::RemapService::pathIsCwd,
    py::doc(R"doc(:class:`bool`: Whether the fix runs from the folder the software started in)doc"))

        .def("clear", &AGRC::RemapService::clear, py::arg("clearLog") = true, py::doc(R"doc(
Clears up all the saved data

Parameters
----------
clearLog: :class:`bool`
    Whether to also clear out any saved data in :attr:`logger`
        )doc"))

        .def("fix", &AGRC::RemapService::fix, py::doc(R"doc(
Fixes every mod found from :attr:`path`

Walks folders depth-first from :attr:`path`, handling every .ini file it finds, and reports what it
did at the end. Writing the log file out afterwards is :class:`RemapServiceCLI`'s
        )doc"));

    // 'stats' is deliberately NOT exposed. AGRemapCore::RemapStats and the Python-facing RemapStats
    // are unrelated C++ types -- PyRemapStats is its own class holding PyFileStats members, not a
    // subclass of the core one -- so the service's own stats cannot cross as the bound RemapStats
    // without unifying those two hierarchies first (the same job the assets layer needed, and got).
    // Nothing needs it yet: fix() already reports its own summary before returning.
}
