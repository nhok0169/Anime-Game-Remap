#include "PyIbFile.h"

#include <string>

#include "AGRemapCore/model/files/BufFileErrors.h"
#include "AGRemapCore/model/files/IbFile.h"
#include "../buffers/PyBufBytes.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    // Same module-path derivation as PyBufFile.cpp/PyBlendFile.cpp's own copies -- see
    // PyBufFile.cpp's comment on why each binding file derives this independently.
    std::string &ibFileErrorsParentPackage() {
        static std::string path;
        return path;
    }

    [[noreturn]] void raisePyBadBufData(const AGRC::BadBufData &e) {
        py::object cls = py::module_::import((ibFileErrorsParentPackage() + ".exceptions.BadBufData").c_str()).attr("BadBufData");
        py::object excInstance = cls(py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    [[noreturn]] void raisePyBufFileNotRecognized(const AGRC::BufFileNotRecognized &e) {
        py::object cls = py::module_::import((ibFileErrorsParentPackage() + ".exceptions.BufFileNotRecognized").c_str()).attr("BufFileNotRecognized");
        py::object excInstance = cls(py::cast(e.filePath()), py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    template <typename Func>
    auto translateBufFileErrors(Func &&func) -> decltype(func()) {
        try {
            return func();
        } catch (const AGRC::BadBufData &e) {
            raisePyBadBufData(e);
        } catch (const AGRC::BufFileNotRecognized &e) {
            raisePyBufFileNotRecognized(e);
        }
    }

}


void initCppIbFile(pybind11::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    ibFileErrorsParentPackage() = parentPackage;

    py::class_<AGRC::IbFile, AGRC::BufFile, py::smart_holder> ibFile(m, "IbFile", R"doc(
This class inherits from :class:`CppBufFile`

Used for handling ``.ib`` (index buffer) files

.. note::
    Where a ``.buf`` file is split into *vertex lines*, a ``.ib`` file is split into *face lines* --
    each line names the vertices making up one triangular face of the mod's mesh. Every face in a
    3dmigoto mod is atomically a triangle, so a line is always :attr:`VerticesPerTriangle` 32-bit
    unsigned integers

Parameters
----------
src: Union[:class:`str`, :class:`bytes`]
    The source file or bytes for the ``.ib`` file

Raises
------
:class:`BufFileNotRecognized`
    If 'src' holds a file path that cannot be read as a valid ``.ib`` file

:class:`BadBufData`
    If 'src' holds raw bytes that are not valid for a ``.ib`` file
    )doc");

    ibFile.def(py::init([](py::object src) {
        return translateBufFileErrors([&]() {
            return std::make_unique<AGRC::IbFile>(pyToBinarySrc(src));
        });
    }), py::arg("src"));

    ibFile.attr("TriangleBufElementKey") = AGRC::IbFile::TriangleBufElementKey;
    ibFile.attr("VerticesPerTriangle") = AGRC::IbFile::VerticesPerTriangle;

    ibFile.def("getTriangleCount", &AGRC::IbFile::getTriangleCount, py::doc(R"doc(
Retrieves the number of triangular faces making up the mod's mesh

Returns
-------
:class:`int`
    The number of faces
    )doc"));

    ibFile.def("getIndexCount", &AGRC::IbFile::getIndexCount, py::doc(R"doc(
Retrieves the number of vertex indices in the file -- :attr:`VerticesPerTriangle` times
:meth:`getTriangleCount`

Returns
-------
:class:`int`
    The number of indices
    )doc"));

    ibFile.def("makeDumpHeader", &AGRC::IbFile::makeDumpHeader, py::arg("firstIndex") = 0, py::doc(R"doc(
Makes the header for a dumped *ib.txt* file

Parameters
----------
firstIndex: :class:`int`
    The index this file's first vertex index is numbered from :raw-html:`<br />` :raw-html:`<br />`

    A mod's faces are spread over several ``.ib`` files (one per mod object), which a dump numbers
    continuously -- so each file after the first starts where the previous one's
    :meth:`getIndexCount` left off :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``0``

Returns
-------
:class:`str`
    The header text
    )doc"));

    ibFile.def("getDumpStr", &AGRC::IbFile::getDumpStr, py::arg("firstIndex") = 0, py::doc(R"doc(
Retrieves the full text for converting this ``.ib`` file into a dumped *ib.txt* file

.. note::
    Unlike :meth:`CppBufFile.getDumpStr`, this returns a *complete* dump -- header included -- and
    its data section comes from :meth:`CppBufFile.getFlatDumpStr`, a ``.ib`` file's own flat,
    space-separated form rather than the per-element form a vertex buffer's data uses:

    .. code-block::

        byte offset: 0
        first index: 0
        index count: 6
        topology: trianglelist
        format: DXGI_FORMAT_R16_UINT

        0 1 2
        3 4 5

Parameters
----------
firstIndex: :class:`int`
    The index this file's first vertex index is numbered from (see :meth:`makeDumpHeader`) :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``0``

Returns
-------
:class:`str`
    The text for the dumped *ib.txt* file
    )doc"));

    ibFile.def("readDumpStr", [](AGRC::IbFile &self, const std::string &text) {
        translateBufFileErrors([&]() {
            self.readDumpStr(text);
            return 0;
        });
    }, py::arg("text"), py::doc(R"doc(
Reads a dumped *ib.txt* file's text back into this ``.ib`` file's bytes -- the inverse of
:meth:`getDumpStr`

.. note::
    This is a convenience for calling :meth:`CppBufFile.readFlatDumpStr` -- an index buffer's dump
    uses the flat, space-separated form, not the per-element form a vertex buffer's does. The header
    is skipped, so a whole dump file can be handed straight in

Parameters
----------
text: :class:`str`
    The text of the dumped *ib.txt* file

Raises
------
:class:`BadBufData`
    If the parsed bytes do not divide evenly into face lines
    )doc"));
}
