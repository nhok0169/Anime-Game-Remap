#include "PyPositionFile.h"

#include <string>

#include "AGRemapCore/model/files/BufFileErrors.h"
#include "AGRemapCore/model/files/PositionFile.h"
#include "../buffers/PyBufBytes.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    // Same module-path derivation as PyBufFile.cpp/PyBlendFile.cpp's own copies -- see
    // PyBufFile.cpp's comment on why each binding file derives this independently.
    std::string &positionFileErrorsParentPackage() {
        static std::string path;
        return path;
    }

    [[noreturn]] void raisePyBadBufData(const AGRC::BadBufData &e) {
        py::object cls = py::module_::import((positionFileErrorsParentPackage() + ".exceptions.BadBufData").c_str()).attr("BadBufData");
        py::object excInstance = cls(py::arg("fileType") = py::cast(e.fileType()));
        PyErr_SetObject(cls.ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    [[noreturn]] void raisePyBufFileNotRecognized(const AGRC::BufFileNotRecognized &e) {
        py::object cls = py::module_::import((positionFileErrorsParentPackage() + ".exceptions.BufFileNotRecognized").c_str()).attr("BufFileNotRecognized");
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


void initCppPositionFile(pybind11::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    positionFileErrorsParentPackage() = parentPackage;

    py::class_<AGRC::PositionFile, AGRC::BufFile, py::smart_holder>(m, "PositionFile", R"doc(
This class inherits from :class:`CppBufFile`

Used for handling ``Position.buf`` files

.. note::
    We observe that a ``Position.buf`` file is a binary file defined as:

    * a line corresponds to the data for a particular vertex in the mod
    * each line contains 40 bytes (320 bits)
    * each line uses little-endian mode (MSB is to the right while LSB is to the left)
    * the first 12 bytes of a line are the coordinate position of a vertex in an R3 vector space, each scalar value in the coordinate is 4 bytes or 32 bits (3 scalar values/line)
    * the next 12 bytes of a line corresponds to the normal vector of a vertex, each scalar value in the vector is 4 bytes or 32 bits (3 scalar values/line)
    * the last 16 bytes of a line corresponds to the tangent vector of a vertex, each scalar value in the vector is 4 bytes or 32 bits (4 scalar values/line)
    * all scalar values in the file are `floating point`_ values
    )doc")

        .def(py::init([](py::object src) {
            return translateBufFileErrors([&]() {
                return std::make_unique<AGRC::PositionFile>(pyToBinarySrc(src));
            });
        }), py::arg("src"), py::doc(R"doc(
Constructs a new position file and immediately reads it

Parameters
----------
src: Union[:class:`str`, :class:`bytes`]
    The source file or bytes for the ``.buf`` file

Raises
------
:class:`BufFileNotRecognized`
    If 'src' holds a file path that cannot be read as a valid position file

:class:`BadBufData`
    If 'src' holds raw bytes that are not valid for a position file
        )doc"));
}
