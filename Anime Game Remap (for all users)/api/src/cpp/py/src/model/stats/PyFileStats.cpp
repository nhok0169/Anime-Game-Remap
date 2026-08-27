#include "PyFileStats.h"

#include <filesystem>

#include <pybind11/stl.h>

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void PyFileStats::clear() {
    AGRC::FileStats::clear();
    skipped.clear();
    skippedByMods.clear();
}

void PyFileStats::addSkipped(const std::string& filePath, py::object error, std::optional<std::string> modFolder) {
    std::string resolvedModFolder = modFolder.has_value() ? *modFolder : std::filesystem::path(filePath).parent_path().string();

    skipped[filePath] = error;
    skippedByMods[resolvedModFolder][filePath] = error;
}

void PyFileStats::updateSkipped(const std::unordered_map<std::string, py::object>& newSkipped, std::optional<std::string> modFolder) {
    if (modFolder.has_value()) {
        for (const auto& entry : newSkipped) {
            skipped[entry.first] = entry.second;
        }

        if (!newSkipped.empty()) {
            auto& modSkipped = skippedByMods[*modFolder];
            for (const auto& entry : newSkipped) {
                modSkipped[entry.first] = entry.second;
            }
        }

        return;
    }

    for (const auto& entry : newSkipped) {
        addSkipped(entry.first, entry.second, std::nullopt);
    }
}

void PyFileStats::update(std::optional<std::string> modFolder, std::optional<std::unordered_set<std::string>> newFixed,
                          std::optional<std::unordered_map<std::string, py::object>> newSkipped,
                          std::optional<std::unordered_set<std::string>> newRemoved,
                          std::optional<std::unordered_set<std::string>> newUndoed,
                          std::optional<std::unordered_set<std::string>> newVisitedAtRemoval) {
    if (newFixed.has_value()) {
        updateFixed(*newFixed);
    }

    if (newSkipped.has_value()) {
        updateSkipped(*newSkipped, modFolder);
    }

    if (newRemoved.has_value()) {
        updateRemoved(*newRemoved);
    }

    if (newUndoed.has_value()) {
        updateUndoed(*newUndoed);
    }

    if (newVisitedAtRemoval.has_value()) {
        updateVisitedAtRemoval(*newVisitedAtRemoval);
    }
}


void initCppFileStats(pybind11::module_ &m) {
    // Bound directly under "FileStats" (Phase 1 of the Cpp-prefix-then-full-replacement
    // playbook -- see Architecture/CLAUDE.md) -- 'FileStats' already exists as a live pure-Python
    // class (model/stats/FileStats.py) at the time this binding was written.
    py::class_<PyFileStats>(m, "FileStats", R"doc(
Keeps track of different types of files encountered by the program

.. note::
    'skipped'/'skippedByMods' hold real Python exception objects (any :class:`Exception` instance),
    not a C++-level exception type -- this is a Python-facing adaptation of the underlying
    AGRemapCore::FileStats (which stores these as an opaque C++ ``std::exception_ptr`` instead, for
    a pure-C++ caller); see this binding's own source comment for why
    )doc")

        .def(py::init<>())

        .def_readwrite("fixed", &PyFileStats::fixed, py::doc(R"doc(
Set[:class:`str`]: The paths to the fixed files
        )doc"))

        .def_readwrite("skipped", &PyFileStats::skipped, py::doc(R"doc(
Dict[:class:`str`, :class:`Exception`]: The exceptions tied to file paths that were skipped due to errors
        )doc"))

        .def_readwrite("skippedByMods", &PyFileStats::skippedByMods, py::doc(R"doc(
Dict[:class:`str`, Dict[:class:`str`, :class:`Exception`]]: The exceptions tied to file paths that were skipped due to errors, grouped by mod folder path
        )doc"))

        .def_readwrite("removed", &PyFileStats::removed, py::doc(R"doc(
Set[:class:`str`]: The file paths for files that got removed
        )doc"))

        .def_readwrite("undoed", &PyFileStats::undoed, py::doc(R"doc(
Set[:class:`str`]: The file paths for files that got undone to a previous state before the software was run
        )doc"))

        .def_readwrite("visitedAtRemoval", &PyFileStats::visitedAtRemoval, py::doc(R"doc(
Set[:class:`str`]: The file paths for files that got visited when attempting to remove those files
        )doc"))

        .def("clear", &PyFileStats::clear, py::doc(R"doc(
Clears out all saved data about the files
        )doc"))

        .def("updateFixed", &AGRC::FileStats::updateFixed, py::arg("newFixed"), py::doc(R"doc(
Updates the fixed file paths

Parameters
----------
newFixed: Set[:class:`str`]
    The newly added file paths that got fixed
        )doc"))

        .def("addFixed", &AGRC::FileStats::addFixed, py::arg("filePath"), py::doc(R"doc(
Adds a file path to the paths of fixed files

Parameters
----------
filePath: :class:`str`
    the new file path to a fixed file
        )doc"))

        .def("updateSkipped", &PyFileStats::updateSkipped, py::arg("newSkipped"), py::arg("modFolder") = py::none(), py::doc(R"doc(
Updates the file paths that got skipped due to errors

Parameters
----------
newSkipped: Dict[:class:`str`, :class:`Exception`]
    The newly skipped file paths (and their errors), due to errors within a particular mod folder

modFolder: Optional[:class:`str`]
    The folder where the files got skipped. If this is ``None``, the mod folder for each entry in
    'newSkipped' is instead read from that entry's own file path

    **Default**: ``None``
        )doc"))

        .def("addSkipped", &PyFileStats::addSkipped, py::arg("filePath"), py::arg("error"), py::arg("modFolder") = py::none(), py::doc(R"doc(
Adds a new file path to the paths of skipped files

Parameters
----------
filePath: :class:`str`
    the new file path that got skipped

error: :class:`Exception`
    The exception that caused the file to be skipped

modFolder: Optional[:class:`str`]
    The mod folder that contains the file path. If this is ``None``, the mod folder is read from
    'filePath''s own parent directory

    **Default**: ``None``
        )doc"))

        .def("updateRemoved", &AGRC::FileStats::updateRemoved, py::arg("newRemoved"), py::doc(R"doc(
Updates the file paths that got removed

Parameters
----------
newRemoved: Set[:class:`str`]
    The newly updated file paths that got removed
        )doc"))

        .def("addRemoved", &AGRC::FileStats::addRemoved, py::arg("filePath"), py::doc(R"doc(
Adds a new file path that got removed

Parameters
----------
filePath: :class:`str`
    The file path that got removed
        )doc"))

        .def("updateUndoed", &AGRC::FileStats::updateUndoed, py::arg("newUndoed"), py::doc(R"doc(
Updates the file paths whose contents got undone to a previous state before the software was run

Parameters
----------
newUndoed: Set[:class:`str`]
    The newly updated file paths that got their contents undone
        )doc"))

        .def("addUndoed", &AGRC::FileStats::addUndoed, py::arg("filePath"), py::doc(R"doc(
Adds a new file path that got undone

Parameters
----------
filePath: :class:`str`
    The file path that got undone
        )doc"))

        .def("updateVisitedAtRemoval", &AGRC::FileStats::updateVisitedAtRemoval, py::arg("newVisitedAtRemoval"), py::doc(R"doc(
Updates the file paths that got visited when the software attempts to remove those files

Parameters
----------
newVisitedAtRemoval: Set[:class:`str`]
    The newly updated file paths that got visited
        )doc"))

        .def("addVisitedAtRemoval", &AGRC::FileStats::addVisitedAtRemoval, py::arg("filePath"), py::doc(R"doc(
Adds a new file path that got visited when the software attempts to remove the file

Parameters
----------
filePath: :class:`str`
    The file path that got visited
        )doc"))

        .def("update", &PyFileStats::update, py::arg("modFolder") = py::none(), py::arg("newFixed") = py::none(),
             py::arg("newSkipped") = py::none(), py::arg("newRemoved") = py::none(), py::arg("newUndoed") = py::none(),
             py::arg("newVisitedAtRemoval") = py::none(), py::doc(R"doc(
Updates the overall file paths in this class -- see :meth:`updateFixed`, :meth:`updateSkipped`, and
:meth:`updateRemoved` for more details

Parameters
----------
modFolder: Optional[:class:`str`]
    The folder where the files got skipped

    **Default**: ``None``

newFixed: Optional[Set[:class:`str`]]
    The newly updated file paths that got fixed

    **Default**: ``None``

newSkipped: Optional[Dict[:class:`str`, :class:`Exception`]]
    The newly skipped file paths due to errors within a particular mod folder

    **Default**: ``None``

newRemoved: Optional[Set[:class:`str`]]
    The newly updated file paths that got removed

    **Default**: ``None``

newUndoed: Optional[Set[:class:`str`]]
    The newly updated file paths that got their contents undone

    **Default**: ``None``

newVisitedAtRemoval: Optional[Set[:class:`str`]]
    The newly updated file paths that got visited when the software attempts to remove those files

    **Default**: ``None``
        )doc"));
}
