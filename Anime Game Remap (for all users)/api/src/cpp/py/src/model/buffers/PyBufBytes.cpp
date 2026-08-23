#include "PyBufBytes.h"

#include <string>

namespace py = pybind11;
namespace AGRC = AGRemapCore;


AGRC::ByteVec pyBytesToVec(const py::bytes &src) {
    std::string s = src;
    return AGRC::ByteVec(s.begin(), s.end());
}

py::bytes vecToPyBytes(const AGRC::ByteVec &src) {
    return py::bytes(reinterpret_cast<const char*>(src.data()), src.size());
}

py::bytearray vecToPyByteArray(const AGRC::ByteVec &src) {
    return py::bytearray(reinterpret_cast<const char*>(src.data()), src.size());
}

AGRC::BinarySrc pyToBinarySrc(const py::object &src) {
    if (py::isinstance<py::str>(src)) {
        return src.cast<std::string>();
    }
    if (py::isinstance<py::bytes>(src)) {
        return pyBytesToVec(src.cast<py::bytes>());
    }
    throw py::type_error("src must be either a 'str' or a 'bytes' object");
}

py::object binarySrcToPy(const AGRC::BinarySrc &src) {
    if (std::holds_alternative<std::string>(src)) {
        return py::cast(std::get<std::string>(src));
    }
    return vecToPyBytes(std::get<AGRC::ByteVec>(src));
}
