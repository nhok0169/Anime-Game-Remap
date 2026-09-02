#include "PyRemapStats.h"

namespace py = pybind11;


void PyRemapStats::clear() {
    blend.clear();
    position.clear();
    texcoord.clear();
    buf.clear();
    other.clear();
    ini.clear();
    texEdit.clear();
    texAdd.clear();
    download.clear();
}


void initCppRemapStats(pybind11::module_ &m) {
    py::class_<PyRemapStats>(m, "RemapStats", R"doc(
The file stats for the overall remap process
    )doc")

        .def(py::init<>())

        .def_readwrite("blend", &PyRemapStats::blend, py::doc(R"doc(
:class:`FileStats`: Stats about whether some ``Blend.buf`` files got fixed/skipped/removed
        )doc"))

        .def_readwrite("position", &PyRemapStats::position, py::doc(R"doc(
:class:`FileStats`: Stats about whether some ``Position.buf`` files got fixed/skipped/removed
        )doc"))

        .def_readwrite("texcoord", &PyRemapStats::texcoord, py::doc(R"doc(
:class:`FileStats`: Stats about whether some ``Texcoord.buf`` files got fixed/skipped/removed
        )doc"))

        .def_readwrite("buf", &PyRemapStats::buf, py::doc(R"doc(
:class:`FileStats`: Stats about whether some other ``.buf`` files got fixed/skipped/removed
        )doc"))

        .def_readwrite("other", &PyRemapStats::other, py::doc(R"doc(
:class:`FileStats`: Stats about whether some files of no recognized kind got fixed/skipped/removed
        )doc"))

        .def_readwrite("ini", &PyRemapStats::ini, py::doc(R"doc(
:class:`FileStats`: Stats about whether some .ini files got fixed/skipped/undone
        )doc"))

        .def_readwrite("texEdit", &PyRemapStats::texEdit, py::doc(R"doc(
:class:`FileStats`: Stats about whether an existing texture file has been edited/removed
        )doc"))

        .def_readwrite("texAdd", &PyRemapStats::texAdd, py::doc(R"doc(
:class:`FileStats`: Stats about whether a brand new texture file created by this software has been created/removed
        )doc"))

        .def_readwrite("download", &PyRemapStats::download, py::doc(R"doc(
:class:`CachedFileStats`: Stats about whether some downloaded mod files have been recently downloaded/removed
        )doc"))

        .def("clear", &PyRemapStats::clear, py::doc(R"doc(
Clears all the stats for the remap process
        )doc"));
}
