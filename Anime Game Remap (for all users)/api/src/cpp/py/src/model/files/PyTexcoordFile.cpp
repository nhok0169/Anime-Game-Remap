// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "PyTexcoordFile.h"

#include <string>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/model/files/BufFileErrors.h"
#include "AGRemapCore/model/files/TexcoordFile.h"
#include "../buffers/PyBufBytes.h"
#include "PyBufFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    // Same module-path derivation as PyBufFile.cpp/PyBlendFile.cpp's own copies -- see
    // PyBufFile.cpp's comment on why each binding file derives this independently.
    std::string &texcoordFileErrorsParentPackage() {
        static std::string path;
        return path;
    }

    [[noreturn]] void raisePyBadBufData(const AGRC::BadBufData &e) {
        py::object cls = py::module_::import((texcoordFileErrorsParentPackage() + ".exceptions.BadBufData").c_str()).attr("BadBufData");
        py::object excInstance = cls(py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    [[noreturn]] void raisePyBufFileNotRecognized(const AGRC::BufFileNotRecognized &e) {
        py::object cls = py::module_::import((texcoordFileErrorsParentPackage() + ".exceptions.BufFileNotRecognized").c_str()).attr("BufFileNotRecognized");
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


void initCppTexcoordFile(pybind11::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    texcoordFileErrorsParentPackage() = parentPackage;

    py::class_<AGRC::TexcoordFile, AGRC::BufFile, py::smart_holder> cls(m, "TexcoordFile", R"doc(
This class inherits from :class:`CppBufFile`

Used for handling ``Texcoord.buf`` files: one element of UV values a vertex (:meth:`uvElement`), whose first two values
are the vertex's U and V, beside any others the layout declares

A mod's UVs can carry two things a draw does not expect: a value that is not a number, and a U past ``1`` that relies on
the sampler WRAPPING -- a part UV'd into the ``[1, 2)`` tile. :meth:`clean` sets the first to ``0`` and folds the second
back into ``[0, 1)``, which selects the same texel wherever the pass wraps and keeps the part right where it does not
    )doc");

    py::class_<AGRC::TexcoordFile::CleanStats>(cls, "CleanStats", "What :meth:`TexcoordFile.clean` changed")
        .def(py::init<>())
        .def_readwrite("cleared", &AGRC::TexcoordFile::CleanStats::cleared,
                       ":class:`int`: How many values that were not a number it set to ``0``")
        .def_readwrite("folded", &AGRC::TexcoordFile::CleanStats::folded,
                       ":class:`int`: How many U values it folded into ``[0, 1)``");

    cls.def_readonly_static("UVKey", &AGRC::TexcoordFile::UVKey, ":class:`str`: The name :meth:`halfElements` gives the UV element")

        .def(py::init([](py::object src, py::object elements, std::string uvElement) {
            return translateBufFileErrors([&]() {
                std::vector<std::unique_ptr<AGRC::BufElementType>> parsed;
                if (!elements.is_none()) {
                    parsed = parseBufElementTypes(elements);
                }
                return std::make_unique<AGRC::TexcoordFile>(pyToBinarySrc(src), std::move(parsed), std::move(uvElement));
            });
        }), py::arg("src"), py::arg("elements") = py::none(), py::arg("uvElement") = AGRC::TexcoordFile::UVKey, py::doc(R"doc(
Constructs a new texcoord file and immediately reads it

Parameters
----------
src: Union[:class:`str`, :class:`bytes`]
    The source file or bytes for the ``.buf`` file

elements: Optional[List[:class:`BufElementType`]]
    The sequence of elements within the ``.buf`` file. If this is ``None`` or empty, uses :meth:`halfElements` of a
    4-byte stride :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

uvElement: :class:`str`
    The element holding the UVs, U first :raw-html:`<br />` :raw-html:`<br />`

    **Default**: :attr:`UVKey`

Raises
------
:class:`BadBufData`
    If the bytes are not a whole number of vertices
        )doc"))

        .def_static("halfElements", [](std::size_t stride) {
            py::list result;
            for (auto& element : AGRC::TexcoordFile::halfElements(stride)) {
                result.append(py::cast(std::move(element)));
            }
            return result;
        }, py::arg("stride"), py::doc(R"doc(
One element, :attr:`UVKey`, of 'stride' / 2 16-bit halves, encoded to the nearest half (ties to even) -- the rounding a
folded U needs

Parameters
----------
stride: :class:`int`
    The bytes of one vertex

Returns
-------
List[:class:`BufElementType`]
    The elements
        )doc"))

        .def("uvElement", &AGRC::TexcoordFile::uvElement, py::doc(R"doc(
The element holding the UVs

Returns
-------
:class:`str`
    Its name
        )doc"))

        .def("us", &AGRC::TexcoordFile::us, py::doc(R"doc(
Each vertex's U, with a value that is not a number read as the ``0`` :meth:`clean` makes it

Returns
-------
List[:class:`float`]
    The U values
        )doc"))

        .def("straddling", &AGRC::TexcoordFile::straddling, py::arg("indices"), py::doc(R"doc(
For each vertex, whether it is a corner of a triangle whose corners sit in DIFFERENT U tiles. Folding one corner of such
a triangle would widen its U span from a few hundredths to nearly ``1`` and interpolate it backwards across the atlas,
so :meth:`clean` leaves those vertices alone -- which also protects deliberate tiling

Parameters
----------
indices: List[:class:`int`]
    The mesh's index list, three to a triangle -- see :meth:`IbFile.getIndices`. An index past the last vertex skips its
    triangle

Returns
-------
List[:class:`bool`]
    One entry a vertex
        )doc"))

        .def("clean", &AGRC::TexcoordFile::clean, py::arg("keep") = std::vector<bool>{}, py::doc(R"doc(
In place: sets every value of the UV element that is not a number to ``0``, and folds each U at or above ``1`` into
``[0, 1)`` (its fractional part), except at a vertex 'keep' marks. A U below ``0`` is left alone: that is the edge bleed
an authoring tool leaves around a UV island, which a clamping sampler is meant to extend, and folded it would sample the
far side of the atlas

Parameters
----------
keep: List[:class:`bool`]
    For each vertex, whether to leave its U alone -- eg. :meth:`straddling`. A vertex past the list's end is folded
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: none is kept

Returns
-------
:class:`TexcoordFile.CleanStats`
    What it changed
        )doc"));
}
