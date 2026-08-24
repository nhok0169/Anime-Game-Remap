#include "PyColourRange.h"

#include "AGRemapCore/model/textures/Colour.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


std::optional<AGRC::ColourOrRangeSet> parseColourOrRangeSet(const py::object &obj) {
    if (obj.is_none()) {
        return std::nullopt;
    }

    AGRC::ColourOrRangeSet result;
    for (auto item : obj) {
        py::object entry = py::reinterpret_borrow<py::object>(item);
        if (py::isinstance<AGRC::ColourRange>(entry)) {
            result.push_back(entry.cast<AGRC::ColourRange>());
        } else {
            result.push_back(entry.cast<AGRC::Colour>());
        }
    }
    return result;
}


py::object colourOrRangeSetToPy(const std::optional<AGRC::ColourOrRangeSet> &colourOrRangeSet) {
    if (!colourOrRangeSet.has_value()) {
        return py::none();
    }

    py::set result;
    for (const auto &entry : *colourOrRangeSet) {
        if (std::holds_alternative<AGRC::Colour>(entry)) {
            result.add(py::cast(std::get<AGRC::Colour>(entry)));
        } else {
            result.add(py::cast(std::get<AGRC::ColourRange>(entry)));
        }
    }
    return std::move(result);
}


void initCppColourRange(pybind11::module_ &m) {
    py::class_<AGRC::ColourRange>(m, "CppColourRange", R"doc(
Class to store a range for a colour

.. container:: operations

    **Supported Operations:**

    .. describe:: hash(x)

        Retrieves the hash id for the colour range based off :meth:`getId`
    )doc")

        .def(py::init<AGRC::Colour, AGRC::Colour>(), py::arg("min"), py::arg("max"), py::doc(R"doc(
Constructs a new colour range

Parameters
----------
min: :class:`CppColour`
    The minimum range for the RGBA values

max: :class:`CppColour`
    The maximum range for the RGBA values
        )doc"))

        .def_readwrite("min", &AGRC::ColourRange::min, py::doc(R"doc(
:class:`CppColour`: The minimum range for the RGBA values
        )doc"))

        .def_readwrite("max", &AGRC::ColourRange::max, py::doc(R"doc(
:class:`CppColour`: The maximum range for the RGBA values
        )doc"))

        // Same reasoning as CppColour's own __hash__ binding: reproduce the pure-Python original's
        // 'return hash(self.getId())' exactly, rather than exposing the C++ std::hash value.
        .def("__hash__", [](const AGRC::ColourRange &self) {
            return py::hash(py::cast(self.getId()));
        })

        .def("getId", &AGRC::ColourRange::getId, py::doc(R"doc(
Retrieves a unique id for the colour range

.. note::
    The id generated will not correspond to any id generated for a single colour

Returns
-------
:class:`str`
    The id for the colour range
        )doc"))

        .def("match", &AGRC::ColourRange::match, py::arg("colour"), py::doc(R"doc(
Whether 'colour' is within the colour range

Parameters
----------
colour: :class:`CppColour`
    The colour to check

Returns
-------
:class:`bool`
    Whether the colour is within the colour range
        )doc"));
}
