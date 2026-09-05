#include "PyIniFile.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <pybind11/stl.h>

#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>

#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "../PyVersion.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// DownloadMode is a plain C++ enum here but a StrEnum on the Python side, so it crosses by value
// rather than by identity -- the same treatment parseGraphReplaceMode gives IniGraphReplaceMode
// (PyResEdit.cpp). Reading through '.value' rather than comparing members means a caller can pass
// either the Python enum member or the bare string it carries.
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


// getResources()/getFileDownloads() hand out unique_ptrs the .ini file owns. Copying them out is
// impossible and moving them out would gut the file, so the binding hands Python borrowed pointers
// and leans on return_value_policy::reference_internal to keep the owning IniFile alive for as long
// as any of them are reachable.
std::vector<AGRC::IniResource*> borrowAll(std::vector<std::unique_ptr<AGRC::IniResource>> &resources) {
    std::vector<AGRC::IniResource*> result;
    result.reserve(resources.size());

    for (std::unique_ptr<AGRC::IniResource> &resource : resources) {
        result.push_back(resource.get());
    }

    return result;
}

}


void initCppIniFile(pybind11::module_ &m) {
    py::class_<AGRC::IniFile>(m, "IniFile", R"doc(
Class for handling .ini files -- the C++-backed counterpart to the pure-Python :class:`IniFile`
:raw-html:`<br />` :raw-html:`<br />`

.. note::
    Mod types cross this boundary as **ids**, not as pure-Python :class:`ModType` objects: this
    class resolves a mod type's parse/fix/remove builders through the global registry keyed by
    ``modTypeId``, or through whatever ``overrideModTypes`` files under that id. See
    :class:`ModType`

Parameters
----------
file: Optional[:class:`str`]
    The file path to the .ini file. If ``None``, this object is backed only by 'txt' and never
    touches the disk :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

txt: :class:`str`
    The text content of the .ini file, used when 'file' is ``None`` :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``""``

gameTypeId: Optional[:class:`int`]
    The id for the game the .ini file's mod belongs to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

filteredFromModTypeIds: Optional[Set[:class:`int`]]
    The ids of the only mod types this .ini file is allowed to be classified as :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``None``, meaning no filter

forcedFromModTypeIds: Optional[Set[:class:`int`]]
    The ids of the mod types this .ini file is classified as regardless of what its content says
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

overrideModTypes: Optional[Dict[:class:`int`, :class:`ModType`]]
    Mod types to resolve by id ahead of the global registry :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

iniClassifier: Optional[:class:`BaseIniClassifier`]
    The strategy used to classify the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the global default classifier

downloadMode: Optional[:class:`DownloadMode`]
    How file downloads referenced by the .ini file are handled :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``None``, meaning :attr:`DownloadMode.Normal`

fromVersion: Optional[:class:`CppVersion`]
    The version of the mod being fixed :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

toVersion: Optional[:class:`CppVersion`]
    The version of the mod being fixed to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

filteredToModTypeIds: Optional[Set[:class:`int`]]
    The ids of the only mod types this .ini file is allowed to be remapped onto :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``None``, meaning no filter
    )doc")

        .def(py::init([](std::optional<std::string> file, std::string txt, std::optional<int> gameTypeId,
                         std::optional<std::unordered_set<int>> filteredFromModTypeIds,
                         std::optional<std::unordered_set<int>> forcedFromModTypeIds,
                         std::optional<std::unordered_map<int, AGRC::ModType>> overrideModTypes,
                         AGRC::BaseIniClassifier *iniClassifier, const py::object &downloadMode,
                         std::optional<AGRC::Version> fromVersion, std::optional<AGRC::Version> toVersion,
                         std::optional<std::unordered_set<int>> filteredToModTypeIds) {
            // 'parseData' is deliberately not a parameter: it is a
            // Dict[int, List[IniGraphGroup<std::string, std::string>]], and the IniGraphGroup bound
            // to Python is a separate pybind-layer class holding a py::dict -- not that
            // instantiation -- so there is no value a Python caller could pass here. Parse data
            // stays on the C++ side, produced by parse() and consumed by fix().
            return std::make_unique<AGRC::IniFile>(std::move(file), std::move(txt), gameTypeId,
                                                   std::move(filteredFromModTypeIds), std::move(forcedFromModTypeIds),
                                                   std::move(overrideModTypes), iniClassifier, std::nullopt,
                                                   parseDownloadMode(downloadMode), std::move(fromVersion),
                                                   std::move(toVersion), std::move(filteredToModTypeIds));
        }), py::arg("file") = py::none(), py::arg("txt") = "", py::arg("gameTypeId") = py::none(),
            py::arg("filteredFromModTypeIds") = py::none(), py::arg("forcedFromModTypeIds") = py::none(),
            py::arg("overrideModTypes") = py::none(), py::arg("iniClassifier") = nullptr,
            py::arg("downloadMode") = py::none(), py::arg("fromVersion") = py::none(),
            py::arg("toVersion") = py::none(), py::arg("filteredToModTypeIds") = py::none(),
            // The classifier is borrowed, not owned -- keep whatever Python object was passed alive
            // for at least as long as this .ini file.
            py::keep_alive<1, 8>())

        // ================================================
        // ================== file identity ===============

        .def_property_readonly("file", &AGRC::IniFile::getFile, py::doc(R"doc(
Optional[:class:`str`]: The file path to the .ini file, or ``None`` if this object is backed only
by its text
        )doc"))

        .def_property_readonly("folder", &AGRC::IniFile::getFolder, py::doc(R"doc(
:class:`str`: The folder the .ini file resides in, or ``""`` when it has no path

.. note::
    This deliberately differs from the pure-Python :attr:`IniFile.folder`, which falls back to the
    folder the script is run from. Derived from :attr:`IniFile.file` rather than stored
        )doc"))

        .def_property("fileTxt", &AGRC::IniFile::getFileTxt, &AGRC::IniFile::setFileTxt, py::doc(R"doc(
:class:`str`: The text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`

Setting this re-reads the text lines from the new value and marks the file as not fixed
        )doc"))

        .def_property_readonly("fileLines", &AGRC::IniFile::getFileLines, py::doc(R"doc(
List[:class:`str`]: The text lines of the .ini file, each keeping its own line ending
        )doc"))

        .def_property_readonly("fileLinesRead", &AGRC::IniFile::fileLinesRead, py::doc(R"doc(
:class:`bool`: Whether the .ini file has been read
        )doc"))

        .def_property_readonly("isModIni", &AGRC::IniFile::getIsMod, py::doc(R"doc(
:class:`bool`: Whether the .ini file belongs to a mod -- the result of :meth:`classify`
        )doc"))

        .def_property("isFixed", &AGRC::IniFile::getIsFixed, &AGRC::IniFile::setIsFixed, py::doc(R"doc(
:class:`bool`: Whether the .ini file has already been fixed
        )doc"))

        .def_property_readonly("isClassified", &AGRC::IniFile::isClassified, py::doc(R"doc(
:class:`bool`: Whether the type of mod has already been identified for the .ini file
        )doc"))

        // A Python-only convenience with no C++ counterpart any more: AGRemapCore::IniFile's
        // own getAvailableType() is gone, because an IniFile can classify as several mod types and
        // "the first one" is a guess rather than an answer. It survives here because the singular
        // shape is still what the Python-side callers want -- Mod.py, and the four Py*Context
        // implementations whose seam (IniParseContext/IniFixContext::modType) takes exactly one
        // mod type by design.
        .def_property_readonly("availableType", [](AGRC::IniFile &self) -> const AGRC::ModType* {
            const tsl::ordered_map<int, AGRC::ModType> &modTypes = self.getModTypes();
            if (modTypes.empty()) {
                return nullptr;
            }

            return &modTypes.begin()->second;
        }, py::return_value_policy::reference_internal, py::doc(R"doc(
Optional[:class:`ModType`]: The type of mod the .ini file was classified as, or ``None`` if it
was not classified as any

.. warning::
    A .ini file can classify as **more than one** mod type, and this answers with whichever comes
    first in :meth:`getModTypes`' insertion order. Use :meth:`getModTypes` whenever "all of them" is
    the right question -- which it usually is. This exists for the callers that genuinely want a
    single mod type
        )doc"))

        // ================================================
        // ==================== reading ===================

        .def("readFileLines", &AGRC::IniFile::readFileLines, py::doc(R"doc(
Reads the text lines of the .ini file off disk

Returns
-------
List[:class:`str`]
    The text lines read, each keeping its own line ending
        )doc"))

        .def("write", &AGRC::IniFile::write, py::arg("txt") = py::none(), py::doc(R"doc(
Writes text back out to the .ini file

Parameters
----------
txt: Optional[:class:`str`]
    The text to write. If ``None``, writes this object's current
    :attr:`IniFile.fileTxt` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`str`
    The text that was written
        )doc"))

        .def("disableIni", &AGRC::IniFile::disableIni, py::arg("makeCopy") = false, py::doc(R"doc(
Disables the .ini file by renaming it so the mod loader stops reading it

Parameters
----------
makeCopy: :class:`bool`
    Whether to keep a copy of the file at its original path :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
Optional[:class:`str`]
    The path the file was moved to, or ``None`` if there was no file to disable
        )doc"))

        // ================================================
        // ================ the fix pipeline ==============

        .def("classify", &AGRC::IniFile::classify, py::doc(R"doc(
Classifies the .ini file -- whether it belongs to a mod, which types of mod, and whether it has
already been fixed :raw-html:`<br />` :raw-html:`<br />`

The results are read back off :attr:`IniFile.isModIni`, :attr:`IniFile.availableType` and
:attr:`IniFile.isFixed`
        )doc"))

        .def("parse", [](AGRC::IniFile &self, bool flushIfTemplates) {
            // Returns None rather than the ParseData it produces: see the constructor's note on
            // why that type cannot cross into Python. The parsed resources are readable through
            // getResources()/getFileDownloads() instead, which is what the parse is run for.
            self.parse(flushIfTemplates);
        }, py::arg("flushIfTemplates") = true, py::doc(R"doc(
Parses the .ini file, building up the resources it references

.. note::
    The parsed graph groups themselves stay on the C++ side. Read the results off
    :meth:`getResources` / :meth:`getFileDownloads`

Parameters
----------
flushIfTemplates: :class:`bool`
    Whether to re-read the `sections`_ of the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
        )doc"))

        .def("fix", &AGRC::IniFile::fix, py::arg("keepBackup") = true, py::arg("fixOnly") = false,
             py::arg("hideOrig") = false, py::doc(R"doc(
Fixes the .ini file, running one fixer per mod type the file was classified as

Parameters
----------
keepBackup: :class:`bool`
    Whether to keep a backup copy of the original .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

fixOnly: :class:`bool`
    Whether to only fix the .ini file without removing any previous fix :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``False``

hideOrig: :class:`bool`
    Whether to comment out the `sections`_ the fix touched, so only the remapped mod displays
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
Dict[:class:`str`, :class:`str`]
    The fix produced for each .ini file written, keyed by file path
        )doc"))

        // The core's 4th parameter ('removedResources', an out-collector of unique_ptr-owned
        // resources) is deliberately not exposed: it hands out ownership of C++-side objects, which
        // is what RemapService consumes it for. A Python caller gets the file's new content, as
        // before.
        .def("removeFix", [](AGRC::IniFile &self, bool parse, bool writeBack, bool readAllIni,
                             bool keepBackups) {
            return self.removeFix(parse, writeBack, readAllIni, keepBackups);
        }, py::arg("parse") = false, py::arg("writeBack") = true,
             py::arg("readAllIni") = false, py::arg("keepBackups") = true, py::doc(R"doc(
Removes a previous fix from the .ini file

Parameters
----------
parse: :class:`bool`
    Whether to parse the .ini file first :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

writeBack: :class:`bool`
    Whether to write the result back out to disk :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

readAllIni: :class:`bool`
    Whether the caller is removing the fix from every .ini file it encountered rather than only the
    ones it could recognize -- :attr:`RemapService.readAllInis` / the script's ``--all`` flag
    :raw-html:`<br />` :raw-html:`<br />`

    A .ini file that belongs to a mod but was not attributed to any type of mod is swept by a
    :class:`GlobalRemapIniRemover` when this is set, and by the ordinary remover when it is not. It does
    not decide *whether* the fix is removed :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
:class:`str`
    The text of the .ini file with the fix removed
        )doc"))

        // ================================================
        // ==================== sections ==================

        .def("getSectionNames", &AGRC::IniFile::getSectionNames, py::doc(R"doc(
Retrieves the names of all the `sections`_ in the .ini file, in the order they are declared

Returns
-------
List[:class:`str`]
    The names of the `sections`_
        )doc"))

        .def("getSection", &AGRC::IniFile::getSection, py::arg("name"),
             py::return_value_policy::reference_internal, py::doc(R"doc(
Retrieves a `section`_ of the .ini file by name

Parameters
----------
name: :class:`str`
    The name of the `section`_

Returns
-------
Optional[:class:`IfTemplate`]
    The `section`_, or ``None`` if no `section`_ goes by that name
        )doc"))

        .def("removeSection", &AGRC::IniFile::removeSection, py::arg("name"), py::doc(R"doc(
Removes a `section`_ from the .ini file, keeping the declaration order of the rest

Parameters
----------
name: :class:`str`
    The name of the `section`_ to remove
        )doc"))

        // ================================================
        // ==================== resources =================

        // --- the surface a drop-in for the pure-Python IniFile needs -------------------------
        // The pure-Python original calls this one 'version'; core splits it in two, since a fix
        // reads from one game version and writes to another.
        // Properties rather than def_readwrite so these accept the same
        // Union[str, int, float, CppVersion] every other version argument in this API does --
        // CppVersion itself exposes no constructor, so a plain readwrite would be unsettable
        // from Python.
        .def_property("fromVersion",
                      [](const AGRC::IniFile &self) { return self.fromVersion; },
                      [](AGRC::IniFile &self, const py::object &v) { self.fromVersion = parseVersionArg(v); },
    py::doc(R"doc(Optional[:class:`CppVersion`]: The game version the .ini file originates from

Accepts a :class:`str`, :class:`int`, :class:`float` or :class:`CppVersion` when set)doc"))

        .def_property("toVersion",
                      [](const AGRC::IniFile &self) { return self.toVersion; },
                      [](AGRC::IniFile &self, const py::object &v) { self.toVersion = parseVersionArg(v); },
    py::doc(R"doc(Optional[:class:`CppVersion`]: The game version to fix the .ini file to

Accepts a :class:`str`, :class:`int`, :class:`float` or :class:`CppVersion` when set)doc"))

        // A property rather than def_readwrite: AGRC::DownloadMode is not a registered pybind
        // enum (Python's DownloadMode is its own StrEnum), so it crosses as its string value --
        // the same convention the constructor already uses.
        .def_readwrite("logger", &AGRC::IniFile::logger,
    py::doc(R"doc(Optional[:class:`BaseLogger`]: Where this .ini file reports progress and problems

``None`` (the default) means nowhere -- messages are dropped rather than buffered. Set rather than
passed: a .ini file is routinely built before the caller has decided where its output should go)doc"))

        .def_property("downloadMode",
                      [](const AGRC::IniFile &self) { return std::string(downloadModeName(self.downloadMode)); },
                      [](AGRC::IniFile &self, const py::object &mode) { self.downloadMode = parseDownloadMode(mode); },
    py::doc(R"doc(:class:`str`: How the .ini file's referenced downloads are handled

Reads back as the :class:`DownloadMode` string value (``"normal"``, ``"disabled"``, ``"always"``);
accepts either a :class:`DownloadMode` or its value when set)doc"))

        // tsl::ordered_set has no built-in pybind11 type_caster (same story as tsl::ordered_map --
        // see PyIniClassifyStats.cpp) so it crosses by hand. A Python list rather than a set on the
        // way out: this container is ORDERED, and handing back a Python set would throw that order
        // away at the boundary. Any Python iterable is accepted on the way in.
        .def_property("defaultModTypeIds",
            [](const AGRC::IniFile &self) {
                py::list result;
                for (int modTypeId : self.defaultModTypeIds) {
                    result.append(py::int_(modTypeId));
                }
                return result;
            },
            [](AGRC::IniFile &self, const py::object &value) {
                tsl::ordered_set<int> result;
                for (const py::handle &item : value) {
                    result.insert(item.cast<int>());
                }
                self.defaultModTypeIds = std::move(result);
            },
    py::doc(R"doc(List[:class:`int`]: The :class:`ModTypeId` values to fall back on when the classifier recognises nothing

In play in exactly one situation: :meth:`classify` ran the classifier and it recognised **no** mod
type at all, in which case :meth:`getModTypes` is built from these ids instead. Deliberately not in
play when ``forcedFromModTypeIds`` was given (the classifier is never consulted for mod types there),
nor when the classifier *did* recognise a mod type that ``filteredFromModTypeIds`` then rejected
(that would quietly undo the caller's own filter)

Also stops :meth:`classify` forcing :attr:`isModIni` false when a mod-type filter was given and
nothing survived it

Reads back as a **list**, not a set, because the order is meaningful -- it is the order the fallback
mod types land in :meth:`getModTypes`, which :meth:`fix` walks. Accepts any iterable when set)doc"))

        .def_readwrite("filteredToModTypeIds", &AGRC::IniFile::filteredToModTypeIds,
    py::doc(R"doc(Optional[Set[:class:`int`]]: Only fix to these mod types, by :class:`ModTypeId` value

``None`` means no filter -- an **empty set** is deliberately different, and selects nothing)doc"))

        .def("setFileTxt", &AGRC::IniFile::setFileTxt, py::arg("txt"), py::doc(R"doc(
Replaces the .ini file's text content, without touching the file on disk

Parameters
----------
txt: :class:`str`
    The new text content
        )doc"))

        .def("getModTypes", [](AGRC::IniFile &self) {
            py::dict result;
            for (const auto &entry : self.getModTypes()) {
                result[py::cast(entry.first)] = py::cast(self.getModTypes().at(entry.first));
            }
            return result;
        }, py::doc(R"doc(
Retrieves the mod types the .ini file was classified as, keyed by :class:`ModTypeId` value

Returns
-------
Dict[:class:`int`, :class:`ModType`]
    The mod types, in the order they were classified
        )doc"))

        .def("getIfTemplates", [](py::object selfObj, bool flush) {
            auto &self = selfObj.cast<AGRC::IniFile &>();
            py::dict result;
            for (const auto &entry : self.getIfTemplates(flush)) {
                // The sections are owned by this IniFile (unique_ptrs in its own map), so every
                // wrapper handed out has to keep the *Python* IniFile alive for as long as it
                // lives -- reference_internal with 'selfObj' as the parent does exactly that. A
                // plain 'reference' cast here (the previous code) left a dangling section the
                // moment the IniFile was collected while a section, or an IniSectionGraph built
                // over these sections, was still around. Note the return_value_policy on the
                // .def itself can't do this job: it is ignored for a py::dict return value.
                result[py::cast(entry.first)] = py::cast(entry.second.get(),
                                                         py::return_value_policy::reference_internal,
                                                         selfObj);
            }
            return result;
        }, py::arg("flush") = false, py::doc(R"doc(
Retrieves every parsed `section`_ of the .ini file, keyed by section name

.. danger::
    The returned sections are owned by this .ini file. :meth:`clear` destroys them

Parameters
----------
flush: :class:`bool`
    Whether to re-read the sections rather than reuse what was already parsed :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
Dict[:class:`str`, :class:`IfTemplate`]
    The sections, in the order they appear in the file
        )doc"))

        .def("getResources", [](AGRC::IniFile &self) {
            return borrowAll(self.getResources());
        }, py::return_value_policy::reference_internal, py::doc(R"doc(
Retrieves every resource the .ini file references

.. danger::
    The returned resources are owned by this .ini file. :meth:`clear` and :meth:`clearModels`
    destroy them

Returns
-------
List[:class:`IniResource`]
    The resources
        )doc"))

        .def("getFileDownloads", [](AGRC::IniFile &self) {
            return borrowAll(self.getFileDownloads());
        }, py::return_value_policy::reference_internal, py::doc(R"doc(
Retrieves every file download the .ini file references

.. danger::
    Same ownership caveat as :meth:`getResources`

Returns
-------
List[:class:`IniResource`]
    The file downloads
        )doc"))

        .def("getReferencedFolders", &AGRC::IniFile::getReferencedFolders, py::doc(R"doc(
Retrieves all the folders referenced by the .ini file, in the order first seen

The parent folder of each resource's source path, across both :meth:`getResources` and
:meth:`getFileDownloads`, plus the parent folder of the fixed path of every one of those that is
an :class:`IniFixResource`

.. note::
    That second half is a deliberate divergence from the pure-Python original, whose own
    ``getReferencedFolders()`` only ever looked at a resource's *source* side. The fix **writes**
    files to a fixed path, so a folder walk built on this method has to be able to reach that
    folder even when no source path points into it

Returns
-------
List[:class:`str`]
    The absolute paths to all the folders
        )doc"))

        // ================================================
        // ==================== clearing ==================

        .def("clearModels", &AGRC::IniFile::clearModels, py::doc(R"doc(
Clears every resource model built for the .ini file, without clearing the text read in from it

.. note::
    To clear the read text instead, see :meth:`clearRead`
        )doc"))

        .def("clearRead", &AGRC::IniFile::clearRead, py::arg("eraseSourceTxt") = false, py::doc(R"doc(
Clears the text data read in from the .ini file

Parameters
----------
eraseSourceTxt: :class:`bool`
    Whether to also erase the text this object was constructed with, when
    :attr:`IniFile.file` is ``None`` and that text is its only data source :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``False``
        )doc"))

        .def("clear", &AGRC::IniFile::clear, py::arg("eraseSourceTxt") = false, py::doc(R"doc(
Clears all the saved data for the .ini file -- everything :meth:`clearRead` and
:meth:`clearModels` clear, plus the classification results and parsed `sections`_

Parameters
----------
eraseSourceTxt: :class:`bool`
    See :meth:`clearRead` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
        )doc"))

        // ================================================
        // ================ line predicates ===============

        .def_static("isSectionHeaderLine", &AGRC::IniFile::isSectionHeaderLine, py::arg("line"), py::doc(R"doc(
Determines whether a line of text declares a `section`_

Parameters
----------
line: :class:`str`
    The line to check

Returns
-------
:class:`bool`
    Whether the line declares a `section`_
        )doc"))

        .def_static("getSectionNameFromLine", &AGRC::IniFile::getSectionNameFromLine, py::arg("line"), py::doc(R"doc(
Retrieves the name of the `section`_ a line declares

Parameters
----------
line: :class:`str`
    The line to read the name out of

Returns
-------
:class:`str`
    The name of the `section`_
        )doc"));
}
