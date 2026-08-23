#include "PyBinaryFile.h"

#include "AGRemapCore/model/files/BinaryFile.h"
#include "../buffers/PyBufBytes.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBinaryFile(pybind11::module_ &m) {
    // Full replacement of the pure-Python original (api/src/py/FixRaidenBoss2/model/files/BinaryFile.py),
    // now deleted -- registered under the bare name directly. Also folds in the pure-Python
    // original's abstract 'File' base -- see AGRC::BinaryFile's own class-level note for why. The
    // pure-Python 'File' class itself is unrelated and still lives on (TextureFile/IniFile inherit
    // from it independently of this class), so it isn't touched by this replacement.
    py::class_<AGRC::BinaryFile, py::smart_holder>(m, "BinaryFile", R"doc(
A class to handle binary files
    )doc")

        .def(py::init([](py::object src) {
            return std::make_unique<AGRC::BinaryFile>(pyToBinarySrc(src));
        }), py::arg("src"), py::doc(R"doc(
Constructs a new binary file

Parameters
----------
src: Union[:class:`str`, :class:`bytes`]
    The source file or bytes for the file
        )doc"))

        .def_property("src", [](AGRC::BinaryFile &self) {
            return binarySrcToPy(self.getSrc());
        }, [](AGRC::BinaryFile &self, py::object src) {
            self.setSrc(pyToBinarySrc(src));
        }, py::doc(R"doc(
Union[:class:`str`, :class:`bytes`]: The source file or bytes for the file
        )doc"))

        .def_property_readonly("data", [](AGRC::BinaryFile &self) {
            return vecToPyBytes(self.getData());
        }, py::doc(R"doc(
:class:`bytes`: The bytes read in from the source
        )doc"))

        .def("read", [](AGRC::BinaryFile &self) {
            return vecToPyBytes(self.read());
        }, py::doc(R"doc(
Reads the data within a file

Returns
-------
:class:`bytes`
    The read bytes
        )doc"));
}
