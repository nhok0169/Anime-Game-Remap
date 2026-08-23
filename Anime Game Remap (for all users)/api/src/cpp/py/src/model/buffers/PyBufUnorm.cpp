#include "PyBufUnorm.h"

#include <string>

#include "AGRemapCore/model/buffers/BufInt.h"
#include "AGRemapCore/model/buffers/BufUnorm.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBufUnorm(pybind11::module_ &m) {
    // Full replacement of the pure-Python original (api/src/py/FixRaidenBoss2/model/buffers/BufUnorm.py),
    // now deleted -- registered under the bare name directly.
    py::class_<AGRC::BufUnorm, AGRC::BufBaseInt, py::smart_holder>(m, "BufUnorm", R"doc(
This class inherits from :class:`BufBaseInt`

The type definition for an `unsigned normalized integer`_ number within a ``.buf`` file
    )doc")

        .def(py::init<std::string, std::size_t, bool>(),
    py::arg("name"), py::arg("size"), py::arg("isBigEndian") = false, py::doc(R"doc(
Constructs a new `unsigned normalized integer`_ type

Parameters
----------
name: :class:`str`
    The name of the type

size: :class:`int`
    The byte size for the data type

isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``
        )doc"));
}
