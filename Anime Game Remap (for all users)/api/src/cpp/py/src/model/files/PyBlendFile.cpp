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

#include "PyBlendFile.h"

#include <optional>
#include <string>

#include <pybind11/stl.h>

#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/files/BlendFile.h"
#include "AGRemapCore/model/files/BufFileErrors.h"
#include "../buffers/PyBufBytes.h"
#include "PyBufFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;

namespace {
    py::list blendElementsToPy(std::vector<std::unique_ptr<AGRC::BufElementType>> elements) {
        py::list result;
        for (auto& element : elements) {
            result.append(py::cast(std::move(element)));
        }

        return result;
    }
}


namespace {

    // Same module-path derivation as PyBufFile.cpp's own bufFileErrorsParentPackage() -- kept as
    // a separate copy (rather than shared across translation units) since each is derived from
    // its own initCppXxx's 'm' parameter and this file's init runs independently of PyBufFile's.
    std::string &blendFileErrorsParentPackage() {
        static std::string path;
        return path;
    }

    [[noreturn]] void raisePyBadBufData(const AGRC::BadBufData &e) {
        py::object cls = py::module_::import((blendFileErrorsParentPackage() + ".exceptions.BadBufData").c_str()).attr("BadBufData");
        py::object excInstance = cls(py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    [[noreturn]] void raisePyBufFileNotRecognized(const AGRC::BufFileNotRecognized &e) {
        py::object cls = py::module_::import((blendFileErrorsParentPackage() + ".exceptions.BufFileNotRecognized").c_str()).attr("BufFileNotRecognized");
        py::object excInstance = cls(py::cast(e.filePath()), py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    // Wraps BlendFile's constructor (the only call in this file that can reach BufFile::read())
    // -- see PyBufFile.cpp's own translateBufFileErrors for the identical shape applied there.
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

    py::object remapResultToPy(const std::variant<std::monostate, std::string, AGRC::ByteVec> &result) {
        if (std::holds_alternative<std::monostate>(result)) {
            return py::none();
        }
        if (std::holds_alternative<std::string>(result)) {
            return py::cast(std::get<std::string>(result));
        }
        return vecToPyByteArray(std::get<AGRC::ByteVec>(result));
    }

}


void initCppBlendFile(pybind11::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    blendFileErrorsParentPackage() = parentPackage;

    py::class_<AGRC::BlendFile, AGRC::BufFile, py::smart_holder>(m, "BlendFile", R"doc(
This class inherits from :class:`CppBufFile`

Used for handling ``Blend.buf`` files

.. note::
    We observe that a ``Blend.buf`` file is a binary file defined as:

    * a line corresponds to the data for a particular vertex in the mod
    * each line contains 32 bytes (256 bits)
    * each line uses little-endian mode (MSB is to the right while LSB is to the left)
    * the first 16 bytes of a line are for the blend weights, each weight is 4 bytes or 32 bits (4 weights/line)
    * the last 16 bytes of a line are for the corresponding indices for the blend weights, each index is 4 bytes or 32 bits (4 indices/line)
    * the blend weights are floating points while the blend indices are unsigned integers
    )doc")

        .def(py::init([](py::object src, py::object elements) {
            return translateBufFileErrors([&]() {
                std::vector<std::unique_ptr<AGRC::BufElementType>> parsedElements;
                if (!elements.is_none()) {
                    parsedElements = parseBufElementTypes(elements);
                }
                return std::make_unique<AGRC::BlendFile>(pyToBinarySrc(src), std::move(parsedElements));
            });
        }), py::arg("src"), py::arg("elements") = py::none(), py::doc(R"doc(
Constructs a new blend file and immediately reads it

Parameters
----------
src: Union[:class:`str`, :class:`bytes`]
    The source file or bytes for the blend file

elements: Optional[List[:class:`BufElementType`]]
    The sequence of elements within the ``.buf`` file. If this argument is ``None`` or empty, will
    use the elements specified for some GIMI character. **Default**: ``None``

Raises
------
:class:`BufFileNotRecognized`
    If 'src' holds a file path that cannot be read as a valid blend file

:class:`BadBufData`
    If 'src' holds raw bytes that are not valid for a blend file
        )doc"))

        .def_static("getMissingIndicesRemap", &AGRC::BlendFile::getMissingIndicesRemap, py::arg("src"), py::arg("vgRemap"), py::doc(R"doc(
Retrieves the temporary remap for any missing blend indices not included in 'vgRemap'

Parameters
----------
src: Dict[:class:`str`, Union[List[:class:`int`], List[:class:`float`]]]
    The data for the blend weights and the blend indices for a particular vertex

vgRemap: :class:`VGRemap`
    The vertex group remap for correcting the Blend.buf file

Returns
-------
Dict[:class:`int`, :class:`int`]
    The temporary remap for the missing indices. The keys are the missing indices found and the
    values are the temporary remapped values for these missing indices
        )doc"))

        .def_static("remapIndices", &AGRC::BlendFile::remapIndices, py::arg("src"), py::arg("vgRemap"), py::arg("remapMissingIndices") = true, py::doc(R"doc(
Remaps the vertex group indices for a particular line (vertex)

Parameters
----------
src: Dict[:class:`str`, Union[List[:class:`int`], List[:class:`float`]]]
    The data for the blend weights and the blend indices for a particular vertex

vgRemap: :class:`VGRemap`
    The vertex group remap for correcting the Blend.buf file

remapMissingIndices: :class:`bool`
    Whether to deactivate any missing blend indices that cannot be identified. **Default**: ``True``

Returns
-------
Dict[:class:`str`, Union[List[:class:`int`], List[:class:`float`]]]
    The new data for the blend weights/blend indices, with the blend indices remapped
        )doc"))

        .def("remap", [](AGRC::BlendFile &self, const AGRC::VGRemap &vgRemap, py::object fixedBlendFile, bool remapMissingIndices) {
            std::optional<std::string> fixedBlendFileOpt;
            if (!fixedBlendFile.is_none()) {
                fixedBlendFileOpt = fixedBlendFile.cast<std::string>();
            }
            return remapResultToPy(self.remap(vgRemap, fixedBlendFileOpt, remapMissingIndices));
        }, py::arg("vgRemap"), py::arg("fixedBlendFile") = py::none(), py::arg("remapMissingIndices") = true, py::doc(R"doc(
Remaps the blend indices in a ``Blend.buf`` file

Parameters
----------
vgRemap: :class:`VGRemap`
    The vertex group remap for correcting the Blend.buf file

fixedBlendFile: Optional[:class:`str`]
    The file path for the fixed ``Blend.buf`` file. **Default**: ``None``

remapMissingIndices: :class:`bool`
    Whether to deactivate any missing blend indices that cannot be identified. **Default**: ``True``

Returns
-------
Union[Optional[:class:`str`], :class:`bytearray`]
    If ``fixedBlendFile`` is ``None`` and no correction was needed, returns ``None``. If
    ``fixedBlendFile`` is ``None`` and correction was needed, returns the fixed bytes. Otherwise
    returns ``fixedBlendFile`` itself
        )doc"))

        .def_static("byteElements", [](std::size_t influences) {
            return blendElementsToPy(AGRC::BlendFile::byteElements(influences));
        }, py::arg("influences"), py::doc(R"doc(
The elements of a `blend`_ line of one-byte slots: 'influences' one-byte ids under ``BLENDINDICES``, then 'influences'
one-byte weights under ``BLENDWEIGHT`` (formatted ``R8G8B8A8_UINT`` when four wide, else ``R8_UINT``)

Parameters
----------
influences: :class:`int`
    How many (id, weight) slots each vertex carries

Returns
-------
List[:class:`BufElementType`]
    The elements
        )doc"))

        .def_static("idElements", [](std::size_t count, std::size_t idBytes) {
            return blendElementsToPy(AGRC::BlendFile::idElements(count, idBytes));
        }, py::arg("count"), py::arg("idBytes"), py::doc(R"doc(
The elements of a line of ids alone: 'count' unsigned ids of 'idBytes' bytes each (1, 2 or 4) under ``BLENDINDICES``,
and no weights -- eg. a blend's true bone ids kept beside it, or a table of bone ids, one a line

Parameters
----------
count: :class:`int`
    How many ids a line carries

idBytes: :class:`int`
    The bytes of one id

Returns
-------
List[:class:`BufElementType`]
    The elements
        )doc"))

        .def_static("influencesOf", &AGRC::BlendFile::influencesOf, py::arg("blendPath"), py::arg("vertices"),
                    py::arg("bytesPerInfluence") = 2, py::doc(R"doc(
How many (id, weight) slots a vertex of the `blend`_ at 'blendPath' carries, from the file's SIZE: its bytes per vertex
over 'bytesPerInfluence'

Parameters
----------
blendPath: :class:`str`
    The `blend`_ file

vertices: :class:`int`
    How many vertices the mesh has (eg. :meth:`CppBufFile.linesOf` its position buffer)

bytesPerInfluence: :class:`int`
    The bytes of one (id, weight) slot :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``2``, a one-byte id and weight

Returns
-------
Optional[:class:`int`]
    The slots a vertex, or ``None`` when the sizes say nothing: no vertices, a file that does not divide by them, or
    bytes a vertex that do not divide into slots
        )doc"))

        .def_static("defaultElements", [](std::size_t influences) {
            return blendElementsToPy(AGRC::BlendFile::defaultElements(influences));
        }, py::arg("influences") = 4, py::doc(R"doc(
The ordinary `blend`_ layout: 'influences' 32-bit float weights then 'influences' 32-bit signed indices

Parameters
----------
influences: :class:`int`
    How many (weight, index) slots each vertex carries :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``4``

Returns
-------
List[:class:`BufElementType`]
    The elements
        )doc"))

        .def_static("elementsWithInfluences", [](const std::vector<AGRC::BufElementType*> &elements, std::size_t influences) {
            std::vector<std::unique_ptr<AGRC::BufElementType>> owned;
            owned.reserve(elements.size());
            for (const AGRC::BufElementType* element : elements) {
                owned.push_back(std::make_unique<AGRC::BufElementType>(*element));
            }
            return blendElementsToPy(AGRC::BlendFile::elementsWithInfluences(owned, influences));
        }, py::arg("elements"), py::arg("influences"), py::doc(R"doc(
A `blend`_ layout made 'influences' wide: a copy of 'elements' whose weight and index elements each carry 'influences'
of their first data type, the other elements unchanged

Parameters
----------
elements: List[:class:`BufElementType`]
    The layout

influences: :class:`int`
    How many (weight, index) slots each vertex should carry

Returns
-------
List[:class:`BufElementType`]
    The new layout
        )doc"))

        .def("influences", &AGRC::BlendFile::influences, py::doc(R"doc(
How many (weight, index) slots each vertex carries: the shorter of the two elements, or 0 when the file has neither

Returns
-------
:class:`int`
    The number of slots
        )doc"))

        .def("setInfluences", [](AGRC::BlendFile &self, std::size_t influences, std::optional<std::vector<std::vector<long long>>> bones) {
            return self.setInfluences(influences, bones.has_value() ? &*bones : nullptr);
        }, py::arg("influences"), py::arg("bones") = py::none(), py::doc(R"doc(
Makes every vertex carry 'influences' (weight, index) slots, in place

* **Wider**: each vertex keeps its slots and gains new ones at the end: bone 0 at weight 0, which no shader weighs
* **Narrower**: slots of one vertex naming the same bone are merged first, their weights summed onto the first of them;
  then the vertex keeps its 'influences' heaviest slots (a tie goes to the lower slot), their weights scaled so the
  vertex's total is what it was. Integer weights are scaled with rounding, the remainder put on the heaviest slot, and a
  sum is capped at the largest value the weight's type holds; float weights are scaled exactly
* **The same width**: nothing changes

Which bone a slot names is its index -- or, given 'bones', the value at the same slot there: a buffer of ids beside the
blend that names the bones for it. The slots kept are returned, so such a buffer can keep the same ones
(:meth:`keepSlots`). The new bytes are :attr:`data`

Parameters
----------
influences: :class:`int`
    How many (weight, index) slots each vertex should carry

bones: Optional[List[List[:class:`int`]]]
    For each vertex, the bone each slot names, or ``None`` to read it from the indices :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
List[List[Optional[:class:`int`]]]
    For each vertex, the slot each new slot came from -- ``None`` for a new, empty slot
        )doc"))

        .def_static("keepSlots", [](const py::bytes &ids, std::size_t idBytes, std::size_t fromSlots, const AGRC::BlendFile::KeptSlots &kept) {
            return vecToPyBytes(AGRC::BlendFile::keepSlots(pyBytesToVec(ids), idBytes, fromSlots, kept));
        }, py::arg("ids"), py::arg("idBytes"), py::arg("fromSlots"), py::arg("kept"), py::doc(R"doc(
A buffer of ids beside a `blend`_ -- each vertex 'fromSlots' unsigned little-endian ids of 'idBytes' bytes -- rewritten
to the slots :meth:`setInfluences` kept. A new slot is id 0

Parameters
----------
ids: :class:`bytes`
    The buffer

idBytes: :class:`int`
    The bytes of one id

fromSlots: :class:`int`
    How many ids each vertex has in 'ids'

kept: List[List[Optional[:class:`int`]]]
    What :meth:`setInfluences` returned

Returns
-------
:class:`bytes`
    The rewritten buffer
        )doc"));
}
