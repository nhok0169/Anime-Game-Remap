#include "PyIniDownloadModel.h"

#include <memory>
#include <string>
#include <vector>

#include <tsl/ordered_map.h>

#include <pybind11/stl.h>

#include "AGRemapCore/model/iniresources/IniDownloadModel.h"
#include "AGRemapCore/tools/files/FileDownload.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

tsl::ordered_map<int, std::vector<std::string>> intVectorMapFromDict(const py::dict &d) {
    tsl::ordered_map<int, std::vector<std::string>> result;
    for (auto item : d) {
        result.emplace(item.first.cast<int>(), item.second.cast<std::vector<std::string>>());
    }
    return result;
}

// Same disown-on-construction contract as PyIniTexModel.cpp's texEditsFromDict -- real call sites
// (data/FileDownloadData.py-style tables) always build a fresh FileDownload per entry, never
// reuse one instance across multiple owners.
tsl::ordered_map<int, std::vector<std::unique_ptr<AGRC::FileDownload>>> downloadsFromDict(const py::dict &d) {
    tsl::ordered_map<int, std::vector<std::unique_ptr<AGRC::FileDownload>>> result;

    for (auto item : d) {
        std::vector<std::unique_ptr<AGRC::FileDownload>> downloads;
        for (auto downloadItem : py::cast<py::list>(item.second)) {
            py::object download = py::reinterpret_borrow<py::object>(downloadItem);
            downloads.push_back(download.cast<std::unique_ptr<AGRC::FileDownload>>());
        }
        result.emplace(item.first.cast<int>(), std::move(downloads));
    }

    return result;
}

// Same known read-back limitation as PyIniTexModel.cpp's texEditsToDict -- no keep-alive wiring;
// unexercised by any real call site today (IniFile.py's makeDLModel still builds the pure-Python
// IniDownloadModelOld).
py::dict downloadsToDict(const AGRC::IniDownloadModel &self) {
    py::dict d;
    for (const auto &[key, downloads] : self.downloads) {
        py::list downloadList;
        for (const auto &download : downloads) {
            downloadList.append(py::cast(download.get(), py::return_value_policy::reference));
        }
        d[py::int_(key)] = downloadList;
    }
    return d;
}

}


void initCppIniDownloadModel(pybind11::module_ &m) {
    py::class_<AGRC::IniDownloadModel, AGRC::IniSrcResourceModel>(m, "IniDownloadModel", R"doc(
This class inherits from :class:`IniSrcResourceModel`

Contains data about a particular resource to download in the original .ini file
    )doc")

        .def(py::init([](std::string iniFolderPath, const py::dict &paths, const py::dict &downloads) {
            // py::init(factory) returning std::unique_ptr<T> by value -- same
            // neither-copyable-nor-movable reasoning as IniGroupedResource's own binding
            // (downloads is a map of vectors of unique_ptr<FileDownload>).
            return std::make_unique<AGRC::IniDownloadModel>(std::move(iniFolderPath), intVectorMapFromDict(paths), downloadsFromDict(downloads));
        }), py::arg("iniFolderPath"), py::arg("paths"), py::arg("downloads"), py::doc(R"doc(
Constructs new data for a resource to download

Parameters
----------
iniFolderPath: :class:`str`
    The folder path to where the .ini file of the resource is located

paths: Dict[:class:`int`, List[:class:`str`]]
    See :class:`IniSrcResourceModel`'s constructor

downloads: Dict[:class:`int`, List[:class:`FileDownload`]]
    The downloader associated with each file -- the keys are the indices to the
    :class:`IfContentPart` that the resource file appears in the :class:`IfTemplate` for some
    resource, and the values are the downloaders for the files within that :class:`IfContentPart`.
    Ownership of each downloader is transferred into this model
        )doc"))

        .def_property("downloads", &downloadsToDict,
            [](AGRC::IniDownloadModel &self, const py::dict &downloads) { self.downloads = downloadsFromDict(downloads); },
    py::doc(R"doc(Dict[:class:`int`, List[:class:`FileDownload`]]: The downloader associated with each file)doc"));
}
