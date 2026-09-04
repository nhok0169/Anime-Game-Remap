#include "PyVbFile.h"

#include <string>
#include <utility>

#include "AGRemapCore/model/files/BufFileErrors.h"
#include "AGRemapCore/model/files/VbFile.h"
#include "../buffers/PyBufBytes.h"
#include "PyBufFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    // Same module-path derivation as PyBufFile.cpp/PyBlendFile.cpp's own copies -- see
    // PyBufFile.cpp's comment on why each binding file derives this independently.
    std::string &vbFileErrorsParentPackage() {
        static std::string path;
        return path;
    }

    [[noreturn]] void raisePyBadBufData(const AGRC::BadBufData &e) {
        py::object cls = py::module_::import((vbFileErrorsParentPackage() + ".exceptions.BadBufData").c_str()).attr("BadBufData");
        py::object excInstance = cls(py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    [[noreturn]] void raisePyBufFileNotRecognized(const AGRC::BufFileNotRecognized &e) {
        py::object cls = py::module_::import((vbFileErrorsParentPackage() + ".exceptions.BufFileNotRecognized").c_str()).attr("BufFileNotRecognized");
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

    // A vector of owned data types/elements handed to Python as a real list of the bound classes.
    // Each is cast out of its unique_ptr individually, so Python takes ownership of each one --
    // pybind's own stl caster cannot move a vector<unique_ptr<...>> across on its own.
    template <typename T>
    py::list ownedVectorToPy(std::vector<std::unique_ptr<T>> values) {
        py::list result;
        for (auto &value : values) {
            result.append(py::cast(std::move(value)));
        }

        return result;
    }

}


void initCppVbFile(pybind11::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    vbFileErrorsParentPackage() = parentPackage;

    py::class_<AGRC::VbFile, AGRC::BufFile, py::smart_holder> vbFile(m, "VbFile", R"doc(
This class inherits from :class:`CppBufFile`

Used for handling ``.vb`` (vertex buffer) files

.. note::
    A GI character's ``.vb`` data does not live in one file -- it is split across a
    ``Position.buf``, a ``Blend.buf`` and a ``Texcoord.buf``, one line each per vertex. Use
    :meth:`CppBufFile.merge` to stitch such a set back together, which fills in both the bytes and
    the elements:

    .. code-block::

        vbFile = VbFile(b"", [])
        vbFile.merge([PositionFile(positionPath), BlendFile(blendPath), texcoordFile])

Parameters
----------
src: Union[:class:`str`, :class:`bytes`]
    The source file or bytes for the ``.vb`` file

elements: List[:class:`BufElementType`]
    The sequence of elements within a vertex line, in byte order :raw-html:`<br />` :raw-html:`<br />`

    Required rather than defaulted: unlike a :class:`BlendFile` or a :class:`PositionFile`, a
    ``.vb`` file has no single fixed layout -- how many texture coordinates it carries varies by mod
    )doc");

    vbFile.def(py::init([](py::object src, py::object elements) {
        return translateBufFileErrors([&]() {
            std::vector<std::unique_ptr<AGRC::BufElementType>> parsedElements;
            if (!elements.is_none()) {
                parsedElements = parseBufElementTypes(elements);
            }

            return std::make_unique<AGRC::VbFile>(pyToBinarySrc(src), std::move(parsedElements));
        });
    }), py::arg("src"), py::arg("elements"));

    vbFile.def("getVertexCount", &AGRC::VbFile::getVertexCount, py::doc(R"doc(
Retrieves the number of vertices making up the mod's mesh

Returns
-------
:class:`int`
    The number of vertices
    )doc"));

    vbFile.def("makeDumpHeader", &AGRC::VbFile::makeDumpHeader, py::doc(R"doc(
Makes the header for a dumped *vb.txt* file

.. note::
    An element's ``SemanticIndex`` is its occurrence among the elements sharing its
    :attr:`BufElementType.name` -- so the first ``TEXCOORD`` is index 0, the next is 1, and so on,
    which is how 3dmigoto tells several same-named elements apart

Returns
-------
:class:`str`
    The header text, ending with the ``vertex-data:`` marker the data section follows
    )doc"));

    vbFile.def("getDumpStr", &AGRC::VbFile::getDumpStr, py::arg("prefix") = "vb0", py::doc(R"doc(
Retrieves the full text for converting this ``.vb`` file into a dumped *vb.txt* file

.. note::
    Unlike :meth:`CppBufFile.getDumpStr`, this returns a *complete* dump -- it is
    :meth:`makeDumpHeader` followed by the data section the parent class produces

Parameters
----------
prefix: :class:`str`
    The buffer name each entry is prefixed with -- the vertex buffer slot a real dump was taken
    from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``"vb0"``

Returns
-------
:class:`str`
    The text for the dumped *vb.txt* file
    )doc"));

    vbFile.def("readDumpStr", [](AGRC::VbFile &self, const std::string &text) {
        translateBufFileErrors([&]() {
            self.readDumpStr(text);
            return 0;
        });
    }, py::arg("text"), py::doc(R"doc(
Reads a dumped *vb.txt* file's text back into this ``.vb`` file's bytes -- the inverse of
:meth:`getDumpStr`

.. note::
    Unlike :meth:`CppBufFile.readDumpStr`, which encodes against whatever :attr:`elements` the file
    already has, this first rebuilds those elements from the dump's own header (see
    :meth:`parseDumpHeader`) when there is one -- so a dump can be read straight back without being
    told its layout. A header-less text falls through to the current elements

Parameters
----------
text: :class:`str`
    The text of the dumped *vb.txt* file

Raises
------
:class:`BadBufData`
    If the parsed bytes do not divide evenly into vertex lines
    )doc"));

    vbFile.def_static("parseFormatName", [](const std::string &formatName) {
        return ownedVectorToPy(AGRC::VbFile::parseFormatName(formatName));
    }, py::arg("formatName"), py::doc(R"doc(
Builds the data types making up an element from the `DXGI format`_ name a dump's header gives for it

.. note::
    The channels decide *how many* data types there are and how wide each one is, and the suffix
    decides what kind they are -- so ``R32G32B32_FLOAT`` is 3 four-byte floats, ``R8G8B8A8_UNORM``
    is 4 one-byte `unsigned normalized integers`_, and ``R32G32B32A32_SINT`` is 4 four-byte signed
    integers

Parameters
----------
formatName: :class:`str`
    The format name to parse, eg. ``"R32G32B32_FLOAT"``

Returns
-------
List[:class:`BufDataType`]
    The data types making up the element. Empty if the format is not one this understands
    )doc"));

    vbFile.def_static("parseDumpHeader", [](const std::string &text) -> py::object {
        std::vector<std::unique_ptr<AGRC::BufElementType>> result = AGRC::VbFile::parseDumpHeader(text);

        // The core signals "nothing usable" with an empty vector, which is the natural shape for a
        // C++ caller; Python has always been given None for that, so it keeps getting None
        if (result.empty()) {
            return py::none();
        }

        return ownedVectorToPy(std::move(result));
    }, py::arg("text"), py::doc(R"doc(
Builds the elements of a vertex line out of a dumped *vb.txt* file's header

.. note::
    The header names each element and gives its `DXGI format`_, which together are everything a
    ``.vb`` file needs -- so a dump can be read back without being told what its layout was

Parameters
----------
text: :class:`str`
    The text of the dumped *vb.txt* file

Returns
-------
Optional[List[:class:`BufElementType`]]
    The elements the header declares, in byte order, or ``None`` when the text has no header to
    read them from (or when one of its formats could not be parsed)
    )doc"));
}
