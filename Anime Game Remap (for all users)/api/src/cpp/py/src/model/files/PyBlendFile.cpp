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

    py::class_<AGRC::BlendFile, AGRC::BufFile, py::smart_holder>(m, "CppBlendFile", R"doc(
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
        )doc"));
}
