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

#include "PyBufFloat.h"

#include <string>

#include "AGRemapCore/model/buffers/BufDataType.h"
#include "AGRemapCore/model/buffers/BufFloat.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppBufFloat(pybind11::module_ &m) {
    // Full replacement of the pure-Python originals (api/src/py/FixRaidenBoss2/model/buffers/BufFloat.py),
    // now deleted -- registered under their bare names directly.
    py::class_<AGRC::BufBaseFloat, AGRC::BufDataType, py::smart_holder>(m, "BufBaseFloat", R"doc(
This class inherits from :class:`BufDataType`

The type definition for a generic 32-bit IEEE 754 `floating point`_ number within a ``.buf`` file
    )doc")

        .def(py::init<std::string, std::size_t, bool>(),
    py::arg("name"), py::arg("size"), py::arg("isBigEndian") = false, py::doc(R"doc(
Constructs a new `floating point`_ type

Parameters
----------
name: :class:`str`
    The name of the type

size: :class:`int`
    The byte size for the data type

isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``
        )doc"));

    py::class_<AGRC::BufFloat, AGRC::BufBaseFloat, py::smart_holder>(m, "BufFloat", R"doc(
This class inherits from :class:`BufBaseFloat`

The type definition for a 32-bit `floating point`_ number within a ``.buf`` file
    )doc")

        .def(py::init<bool>(), py::arg("isBigEndian") = false, py::doc(R"doc(
Constructs a new 32-bit `floating point`_ type

Parameters
----------
isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``
        )doc"));

    // REGISTERED BEFORE THE CLASS, because it is a default argument of the constructor
    // below and pybind11 resolves those at registration time -- declared after, the whole
    // module raises `arg(): could not convert default argument` on import.
    py::enum_<AGRC::BufFloat16::Rounding>(m, "BufFloat16Rounding", R"doc(
How :class:`BufFloat16` narrows a wider value to a 16-bit half

The two are not interchangeable and the difference is measurable: over one WuWa mod's texcoord
buffer they disagree on **104 halves of 1,508,336**, every one of which is a moved UV.
    )doc")
        .value("Truncate", AGRC::BufFloat16::Rounding::Truncate, R"doc(
Drop the low mantissa bits and flush a subnormal to zero

The behaviour this type has always had, and the default, so that no buffer the library already
writes moves. Cheap, and **not** an exact round trip.
        )doc")
        .value("NearestEven", AGRC::BufFloat16::Rounding::NearestEven, R"doc(
Round the mantissa half to even, keep subnormals, and keep a ``NaN`` a ``NaN``

What numpy's ``float16`` cast does. Under this mode :meth:`BufFloat16.decode` followed by
:meth:`BufFloat16.encode` is an **exact identity** for every one of the 65536 half bit patterns,
which is what makes :meth:`BufFile.fix` -- it re-encodes every line, including the ones no filter
touched -- safe to run over a buffer of halves.
        )doc");

    py::class_<AGRC::BufFloat16, AGRC::BufBaseFloat, py::smart_holder>(m, "BufFloat16", R"doc(
This class inherits from :class:`BufBaseFloat`

The type definition for a 16-bit `half precision floating point`_ number within a ``.buf`` file
    )doc")

        .def(py::init<bool, AGRC::BufFloat16::Rounding>(), py::arg("isBigEndian") = false,
             py::arg("rounding") = AGRC::BufFloat16::Rounding::Truncate, py::doc(R"doc(
Constructs a new 16-bit `half precision floating point`_ type

Parameters
----------
isBigEndian: :class:`bool`
    Whether the type is in big endian mode. **Default**: ``False``

rounding: :class:`BufFloat16Rounding`
    How :meth:`encode` narrows a value to a half. **Default**: ``BufFloat16Rounding.Truncate``
        )doc"))

        .def_property_readonly("rounding", &AGRC::BufFloat16::getRounding, py::doc(R"doc(
How :meth:`encode` narrows a value to a half

:getter: Retrieves the rounding mode
:type: :class:`BufFloat16Rounding`
        )doc"));

}
