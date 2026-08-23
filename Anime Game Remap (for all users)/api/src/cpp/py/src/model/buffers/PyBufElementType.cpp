#include "PyBufElementType.h"

#include <pybind11/stl.h>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufType.h"
#include "PyBufBytes.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


std::vector<std::unique_ptr<AGRC::BufDataType>> parseBufDataTypes(const py::object &dataTypes) {
    // Clones each incoming BufDataType (via its own polymorphic BufDataType::clone()) rather than
    // taking ownership of the exact Python object passed in -- unlike PyIfTemplate.cpp's
    // parsePartsList (which deliberately disowns the original, since an IfTemplatePart is a
    // unique, identity-bearing node), a BufDataType is a small shareable value. This codebase's
    // own real usage relies on that: constants/BufDataTypes.py caches each data type once behind
    // a DeferredEnum and reuses the exact same Python object across many BufElementType
    // definitions (eg. 'BufDataTypes.Float32.value' shared by PositionFloatRGB, NormalFloatRGB,
    // TangentFloatRGBA, ...) -- taking ownership here would disown that shared instance the first
    // time it's used and raise "Python instance was disowned" on every subsequent reuse.
    std::vector<std::unique_ptr<AGRC::BufDataType>> result;
    for (auto item : dataTypes) {
        py::object dataType = py::reinterpret_borrow<py::object>(item);
        const AGRC::BufDataType &ref = dataType.cast<const AGRC::BufDataType&>();
        result.push_back(ref.clone());
    }
    return result;
}


void initCppBufElementType(pybind11::module_ &m) {
    // Full replacement of the pure-Python original (api/src/py/FixRaidenBoss2/model/buffers/BufElementType.py),
    // now deleted -- registered under the bare name directly.
    py::class_<AGRC::BufElementType, AGRC::BufType, py::smart_holder>(m, "BufElementType", R"doc(
This class inherits from :class:`BufType`

The type definition for an element within a ``.buf`` file
    )doc")

        .def(py::init([](std::string name, std::string formatName, py::object dataTypes) {
            return std::make_unique<AGRC::BufElementType>(std::move(name), std::move(formatName), parseBufDataTypes(dataTypes));
        }), py::arg("name"), py::arg("formatName"), py::arg("dataTypes"), py::doc(R"doc(
Constructs a new element type

Parameters
----------
name: :class:`str`
    The name of the element

formatName: :class:`str`
    The name of the type format according to 3dmigoto

dataTypes: List[:class:`BufDataType`]
    The data types composed within the element, in byte order -- each is cloned, so the same
    passed-in instance can safely be reused for other elements afterward
        )doc"))

        .def_property("formatName", &AGRC::BufElementType::getFormatName, &AGRC::BufElementType::setFormatName, py::doc(R"doc(
:class:`str`: The name of the type format according to 3dmigoto
        )doc"))

        .def_property("dataTypes", [](AGRC::BufElementType &self) {
            std::vector<AGRC::BufDataType*> result;
            for (const auto &dataType : self.getDataTypes()) {
                result.push_back(dataType.get());
            }
            return result;
        }, [](AGRC::BufElementType &self, py::object dataTypes) {
            self.setDataTypes(parseBufDataTypes(dataTypes));
        }, py::return_value_policy::reference_internal, py::doc(R"doc(
List[:class:`BufDataType`]: The data types composed within the element

Assigning a new list clones each new data type the same way the constructor's own ``dataTypes``
parameter does
        )doc"))

        .def_property_readonly("size", &AGRC::BufElementType::getSize, py::doc(R"doc(
:class:`int`: The byte size for the element
        )doc"))

        .def("decode", [](const AGRC::BufElementType &self, const py::bytes &src) {
            return self.decode(pyBytesToVec(src));
        }, py::arg("src"), py::doc(R"doc(
Decodes a raw sequence of bytes into one decoded value per data type composing this element

Parameters
----------
src: :class:`bytes`
    The source bytes to decode

Returns
-------
List[Union[:class:`int`, :class:`float`]]
    The decoded values, one per entry of :attr:`dataTypes`, in the same order
        )doc"))

        .def("encode", [](const AGRC::BufElementType &self, const std::vector<AGRC::BufValue> &src) {
            return vecToPyBytes(self.encode(src));
        }, py::arg("src"), py::doc(R"doc(
Encodes the decoded values for this element back to raw bytes

Parameters
----------
src: List[Union[:class:`int`, :class:`float`]]
    The decoded values to encode, one per entry of :attr:`dataTypes`

Returns
-------
:class:`bytes`
    The encoded raw bytes
        )doc"));
}
