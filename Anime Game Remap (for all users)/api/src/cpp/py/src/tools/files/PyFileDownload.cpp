#include "PyFileDownload.h"

#include <pybind11/stl.h>

#include "AGRemapCore/tools/files/FileDownload.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppFileDownload(pybind11::module_ &m) {
    // py::smart_holder (not the default holder) -- required so a unique_ptr<FileDownload> can
    // be extracted from an existing Python object (via pyObj.cast<std::unique_ptr<AGRC::FileDownload>>())
    // when constructing RemapIniDownload/IniDownloadModel, which take ownership of one.
    //
    // No trampoline: 'download' is virtual on the C++ side (so a C++-side test double can override
    // it independently of AGRemapCore::FileDownload::get()'s own caching logic), but nothing in the
    // real Python call sites subclasses FileDownload -- if that's ever needed, get() calling
    // this->download(...) polymorphically means a real PYBIND11_OVERRIDE trampoline would be needed
    // then (not just py::smart_holder), per the "C++ code calling through the base type itself"
    // rule in Architecture/CLAUDE.md.
    py::class_<AGRC::FileDownload, py::smart_holder>(m, "FileDownload", R"doc(
Class to handle file downloads from some server
    )doc")

        .def(py::init<std::string, std::string, bool>(), py::arg("url"), py::arg("filename"), py::arg("cache") = true, py::doc(R"doc(
Constructs a new file download

Parameters
----------
url: :class:`str`
    The link to the file download

filename: :class:`str`
    The base name of the file (with extension)

cache: :class:`bool`
    Whether to copy the previously-downloaded file if possible, instead of downloading another copy

    **Default**: ``True``
        )doc"))

        .def_readwrite("url", &AGRC::FileDownload::url, py::doc(R"doc(
:class:`str`: The link to the file download
        )doc"))

        .def_readwrite("filename", &AGRC::FileDownload::filename, py::doc(R"doc(
:class:`str`: The base name of the file (with extension)
        )doc"))

        .def_readwrite("cache", &AGRC::FileDownload::cache, py::doc(R"doc(
:class:`bool`: Whether to copy the previously-downloaded file if possible, instead of downloading another copy
        )doc"))

        .def("download", &AGRC::FileDownload::download, py::arg("folder"), py::arg("proxy") = py::none(), py::doc(R"doc(
Downloads the required file

Parameters
----------
folder: :class:`str`
    The folder to store the downloaded file (created if it doesn't already exist)

proxy: Optional[:class:`str`]
    The link to the proxy server used for any internet network access, if any

    **Default**: ``None``

Returns
-------
:class:`str`
    The full path to the downloaded file
        )doc"))

        .def("get", &AGRC::FileDownload::get, py::arg("folder"), py::arg("proxy") = py::none(), py::doc(R"doc(
Retrieves the required file -- either from :meth:`download`, or (if 'cache' is ``True`` and a
previous download already exists) by copying the previously-downloaded file instead

Parameters
----------
folder: :class:`str`
    The folder to store the downloaded file

proxy: Optional[:class:`str`]
    The link to the proxy server used for any internet network access, if any

    **Default**: ``None``

Returns
-------
Tuple[:class:`str`, :class:`bool`, :class:`bool`]
    A tuple containing, in order: the path to the downloaded file; whether a download actually
    occurred; whether a previous download to the file already existed before this call
        )doc"));
}
