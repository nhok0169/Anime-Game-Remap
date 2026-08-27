#include "PyCachedFileStats.h"

#include <pybind11/stl.h>

namespace py = pybind11;


void PyCachedFileStats::clear() {
    PyFileStats::clear();
    hit.clear();
}

void PyCachedFileStats::addHit(const std::string& filePath) {
    hit.insert(filePath);
}

void PyCachedFileStats::updateHit(const std::unordered_set<std::string>& newHit) {
    hit.insert(newHit.begin(), newHit.end());
}

void PyCachedFileStats::update(std::optional<std::string> modFolder, std::optional<std::unordered_set<std::string>> newFixed,
                                std::optional<std::unordered_map<std::string, py::object>> newSkipped,
                                std::optional<std::unordered_set<std::string>> newRemoved,
                                std::optional<std::unordered_set<std::string>> newUndoed,
                                std::optional<std::unordered_set<std::string>> newVisitedAtRemoval,
                                std::optional<std::unordered_set<std::string>> newHit) {
    PyFileStats::update(modFolder, newFixed, newSkipped, newRemoved, newUndoed, newVisitedAtRemoval);

    if (newHit.has_value()) {
        updateHit(*newHit);
    }
}


void initCppCachedFileStats(pybind11::module_ &m) {
    py::class_<PyCachedFileStats, PyFileStats>(m, "CachedFileStats", R"doc(
This class inherits from :class:`FileStats`

Adds tracking for a file retrieved via a cache hit, on top of what :class:`FileStats` already tracks
    )doc")

        .def(py::init<>())

        .def_readwrite("hit", &PyCachedFileStats::hit, py::doc(R"doc(
Set[:class:`str`]: The paths to the files retrieved during a cache hit
        )doc"))

        .def("addHit", &PyCachedFileStats::addHit, py::arg("filePath"), py::doc(R"doc(
Adds a new file path to the paths of cache hit files

Parameters
----------
filePath: :class:`str`
    the new file path that was hit
        )doc"))

        .def("updateHit", &PyCachedFileStats::updateHit, py::arg("newHit"), py::doc(R"doc(
Updates the file paths that have a cache hit

Parameters
----------
newHit: Set[:class:`str`]
    The new file paths that got a hit
        )doc"))

        .def("update", &PyCachedFileStats::update, py::arg("modFolder") = py::none(), py::arg("newFixed") = py::none(),
             py::arg("newSkipped") = py::none(), py::arg("newRemoved") = py::none(), py::arg("newUndoed") = py::none(),
             py::arg("newVisitedAtRemoval") = py::none(), py::arg("newHit") = py::none(), py::doc(R"doc(
Same as :meth:`FileStats.update`, with an additional 'newHit' argument

Parameters
----------
newHit: Optional[Set[:class:`str`]]
    The new file paths that got a cache hit

    **Default**: ``None``
        )doc"));
}
