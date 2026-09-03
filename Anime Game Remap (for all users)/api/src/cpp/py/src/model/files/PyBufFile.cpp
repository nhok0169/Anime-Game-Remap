#include "PyBufFile.h"

#include <memory>
#include <optional>
#include <string>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/files/BufFileErrors.h"
#include "../buffers/PyBufBytes.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


std::vector<std::unique_ptr<AGRC::BufElementType>> parseBufElementTypes(const py::object &elements) {
    // Clones each incoming BufElementType (a real C++ copy, deep-cloning its own BufDataTypes in
    // turn) rather than taking ownership of the exact Python object -- see PyBufElementType.cpp's
    // parseBufDataTypes for the identical reasoning one hierarchy level down. A BufElementType is
    // just as shareable in this codebase's real usage: constants/BufElementTypes.py caches each
    // element once behind a DeferredEnum (eg. 'BufElementTypes.BlendWeightFloatRGBA.value'), and a
    // caller can hold one shared 'elements' list config (eg. RemapBlendResource.blendElements)
    // that gets passed into more than one BufFile/BlendFile construction over its lifetime.
    std::vector<std::unique_ptr<AGRC::BufElementType>> result;
    for (auto item : elements) {
        py::object element = py::reinterpret_borrow<py::object>(item);
        const AGRC::BufElementType &ref = element.cast<const AGRC::BufElementType&>();
        result.push_back(std::make_unique<AGRC::BufElementType>(ref));
    }
    return result;
}


namespace {

    // The fully-qualified dotted path to this 'core' module's own parent package -- derived once
    // (see initCppBufFile) the same way PyBaseTokenizer.cpp's syntaxErrModulePath() is, and for
    // the exact same reason (this repo's Unit Tester harness imports the package as
    // 'src.py.FixRaidenBoss2' rather than a top-level 'FixRaidenBoss2').
    std::string &bufFileErrorsParentPackage() {
        static std::string path;
        return path;
    }

    [[noreturn]] void raisePyBadBufData(const AGRC::BadBufData &e) {
        py::object cls = py::module_::import((bufFileErrorsParentPackage() + ".exceptions.BadBufData").c_str()).attr("BadBufData");
        py::object excInstance = cls(py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    [[noreturn]] void raisePyBufFileNotRecognized(const AGRC::BufFileNotRecognized &e) {
        py::object cls = py::module_::import((bufFileErrorsParentPackage() + ".exceptions.BufFileNotRecognized").c_str()).attr("BufFileNotRecognized");
        py::object excInstance = cls(py::cast(e.filePath()), py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    // Wraps any call that can construct/read a BufFile, translating the two plain, Python-free
    // core exceptions into the exact real Python exception classes real callers already catch by
    // name -- mirrors PyBaseTokenizer.cpp's callSimplifiedMaximalMunch.
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


AGRC::BufFile::Filter parseFilter(const py::function &filter) {
    return [filter](const AGRC::BufLineData &data, long long startInd, double lineInd, long long lineSize) {
        py::gil_scoped_acquire gil;
        py::object result = filter(data, startInd, lineInd, lineSize);
        return result.cast<AGRC::BufLineData>();
    };
}

std::vector<AGRC::BufFile::Filter> parseFilters(const std::vector<py::function> &filters) {
    std::vector<AGRC::BufFile::Filter> result;
    result.reserve(filters.size());
    for (const auto &filter : filters) {
        result.push_back(parseFilter(filter));
    }
    return result;
}

py::object fixResultToPy(const AGRC::BufFile::FixResult &result) {
    if (std::holds_alternative<std::string>(result)) {
        return py::cast(std::get<std::string>(result));
    }
    return vecToPyByteArray(std::get<AGRC::ByteVec>(result));
}


AGRC::BufFile::FixResult pyToFixResult(const py::object &result) {
    if (py::isinstance<py::str>(result)) {
        return AGRC::BufFile::FixResult(result.cast<std::string>());
    }

    py::buffer buf = result.cast<py::buffer>();
    py::buffer_info info = buf.request();
    const auto *data = static_cast<const std::uint8_t*>(info.ptr);
    return AGRC::BufFile::FixResult(AGRC::ByteVec(data, data + info.size));
}


void initCppBufFile(pybind11::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    bufFileErrorsParentPackage() = parentPackage;

    py::class_<AGRC::BufFile, AGRC::BinaryFile, py::smart_holder>(m, "CppBufFile", R"doc(
This class inherits from :class:`BinaryFile`

A class to handle ``.buf`` files

A ``.buf`` file is a binary file made up of a sequence of same-sized "lines" (one line per vertex),
each one composed of the same sequence of :class:`BufElementType`\s -- there is no header or
footer, just the lines themselves back-to-back
    )doc")

        .def(py::init([](py::object src, py::object elements, std::string fileType) {
            return translateBufFileErrors([&]() {
                return std::make_unique<AGRC::BufFile>(pyToBinarySrc(src), parseBufElementTypes(elements), std::move(fileType));
            });
        }), py::arg("src"), py::arg("elements"), py::arg("fileType") = "Buffer", py::doc(R"doc(
Constructs a new ``.buf`` file and immediately reads it

Parameters
----------
src: Union[:class:`str`, :class:`bytes`]
    The source file or bytes for the ``.buf`` file

elements: List[:class:`BufElementType`]
    The sequence of elements within the ``.buf`` file -- each is cloned, so the same passed-in
    instance can safely be reused for other ``.buf`` files afterward

fileType: :class:`str`
    The name for the type of ``.buf`` file. **Default**: ``"Buffer"``

Raises
------
:class:`BufFileNotRecognized`
    If 'src' holds a file path that cannot be read as a valid ``.buf`` file of this format

:class:`BadBufData`
    If 'src' holds raw bytes that are not valid for this format
        )doc"))

        .def_property("fileType", &AGRC::BufFile::getFileType, &AGRC::BufFile::setFileType, py::doc(R"doc(
:class:`str`: The name for the type of ``.buf`` file
        )doc"))

        .def_property("elements", [](AGRC::BufFile &self) {
            std::vector<AGRC::BufElementType*> result;
            for (const auto &element : self.getElements()) {
                result.push_back(element.get());
            }
            return result;
        }, [](AGRC::BufFile &self, py::object elements) {
            self.setElements(parseBufElementTypes(elements));
        }, py::return_value_policy::reference_internal, py::doc(R"doc(
List[:class:`BufElementType`]: The sequence of elements within the ``.buf`` file

Assigning a new list clones each new element the same way the constructor's own ``elements``
parameter does
        )doc"))

        .def_property_readonly("bytesPerLine", &AGRC::BufFile::getBytesPerLine, py::doc(R"doc(
:class:`int`: The number of bytes per line (per vertex)
        )doc"))

        .def("isValid", &AGRC::BufFile::isValid, py::doc(R"doc(
Whether the size of the data is divisible by the # of bytes per line

Returns
-------
:class:`bool`
    Whether the provided data for the ``.buf`` file is valid
        )doc"))

        .def("read", [](AGRC::BufFile &self) {
            return translateBufFileErrors([&]() {
                return vecToPyBytes(self.read());
            });
        }, py::doc(R"doc(
Reads the bytes in the ``.buf`` file

Returns
-------
:class:`bytes`
    The read bytes

Raises
------
:class:`BufFileNotRecognized`
    If :attr:`src` holds a file path that cannot be read as a valid ``.buf`` file of this format

:class:`BadBufData`
    If :attr:`src` holds raw bytes that are not valid for this format
        )doc"))

        .def("decodeLine", [](const AGRC::BufFile &self, const py::bytes &src) {
            return self.decodeLine(pyBytesToVec(src));
        }, py::arg("src"), py::doc(R"doc(
Decodes a line (a vertex) within the ``.buf`` file

Parameters
----------
src: :class:`bytes`
    The source bytes to decode

Returns
-------
Dict[:class:`str`, List[Any]]
    The decoded values for the line

    The keys are the names to the elements and the values are what is decoded
        )doc"))

        .def("encodeLine", [](const AGRC::BufFile &self, const AGRC::BufLineData &src) {
            return vecToPyBytes(self.encodeLine(src));
        }, py::arg("src"), py::doc(R"doc(
Encodes the data about a vertex to their corresponding bytes for the line

Parameters
----------
src: Dict[:class:`str`, List[Any]]
    The corresponding data for the vertex

    The keys are the names for the elements and the values are the data for the elements

Returns
-------
:class:`bytes`
    The encoded bytes for the line
        )doc"))

        .def("fix", [](AGRC::BufFile &self, py::object fixedFile, py::object filters) {
            std::optional<std::string> fixedFileOpt;
            if (!fixedFile.is_none()) {
                fixedFileOpt = fixedFile.cast<std::string>();
            }

            std::vector<py::function> rawFilters;
            if (!filters.is_none()) {
                rawFilters = filters.cast<std::vector<py::function>>();
            }

            AGRC::BufFile::FixResult result = self.fix(fixedFileOpt, parseFilters(rawFilters));
            return fixResultToPy(result);
        }, py::arg("fixedFile") = py::none(), py::arg("filters") = py::none(), py::doc(R"doc(
Fixes the ``.buf`` file

Parameters
----------
fixedFile: Optional[:class:`str`]
    The file path for the fixed ``.buf`` file. **Default**: ``None``

filters: Optional[List[Callable[[Dict[:class:`str`, List[Any]], :class:`int`, :class:`int`, :class:`int`], Dict[:class:`str`, List[Any]]]]]
    The filters to process each element, applied in order to each line

    The filters take in the following arguments:

    #. The data for a particular line
    #. The starting byte index of the line that is read
    #. The line index being processed (``i / bytesPerLine`` -- a `floating point`_ value, matching
       this codebase's pure-Python original exactly)
    #. The size of each line

    The output of the filters is the resultant data that consists where the keys are the names of
    the elements within a line in the ``.buf`` file and the values are the resultant data for each
    element in the line. **Default**: ``None``

Returns
-------
Union[Optional[:class:`str`], :class:`bytearray`]
    If the argument ``fixedFile`` is ``None``, then will return an array of bytes for the fixed
    ``.buf`` file. Otherwise will return the filename to the fixed ``.buf`` file
        )doc"));
}
