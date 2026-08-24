#include "PyColour.h"

#include "AGRemapCore/model/textures/Colour.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppColour(pybind11::module_ &m) {
    py::class_<AGRC::Colour>(m, "CppColour", R"doc(
Class to store data for a colour

.. container:: operations

    **Supported Operations:**

    .. describe:: hash(x)

        Retrieves the hash id for the colour based off :meth:`getId`
    )doc")

        .def(py::init<int, int, int, int>(), py::arg("red") = 255, py::arg("green") = 255,
             py::arg("blue") = 255, py::arg("alpha") = 255, py::doc(R"doc(
Constructs a new colour

Parameters
----------
red: :class:`int`
    The red channel for the colour. **Default**: ``255``

green: :class:`int`
    The green channel for the colour. **Default**: ``255``

blue: :class:`int`
    The blue channel for the colour. **Default**: ``255``

alpha: :class:`int`
    The transparency (alpha) channel for the colour, with a range from 0-255. 0 = transparent,
    255 = opaque. **Default**: ``255``
        )doc"))

        .def_readwrite("red", &AGRC::Colour::red, py::doc(R"doc(
:class:`int`: The red channel for the colour
        )doc"))

        .def_readwrite("green", &AGRC::Colour::green, py::doc(R"doc(
:class:`int`: The green channel for the colour
        )doc"))

        .def_readwrite("blue", &AGRC::Colour::blue, py::doc(R"doc(
:class:`int`: The blue channel for the colour
        )doc"))

        .def_readwrite("alpha", &AGRC::Colour::alpha, py::doc(R"doc(
:class:`int`: The transparency (alpha) channel for the colour, with a range from 0-255. 0 =
transparent, 255 = opaque
        )doc"))

        .def_static("boundColourChannel", &AGRC::Colour::boundColourChannel, py::arg("val"),
                     py::arg("min") = 0, py::arg("max") = 255, py::doc(R"doc(
Makes a colour channel value be in between the minimum and maximum value

Parameters
----------
val: :class:`int`
    The value of the channel

min: :class:`int`
    The minimum bound for the colour channel. **Default**: ``0``

max: :class:`int`
    The maximum bound for the colour channel. **Default**: ``255``

Returns
-------
:class:`int`
    The bounded value
        )doc"))

        .def_static("boolToColourChannel", &AGRC::Colour::boolToColourChannel, py::arg("val"),
                     py::arg("min") = 0, py::arg("max") = 255, py::doc(R"doc(
Converts a boolean value to a value for a colour channel

Parameters
----------
val: :class:`bool`
    The boolean value to convert

min: :class:`int`
    The minimum bound for the colour channel. **Default**: ``0``

max: :class:`int`
    The maximum bound for the colour channel. **Default**: ``255``

Returns
-------
:class:`int`
    The corresponding value for the colour channel based off the boolean
        )doc"))

        // Deliberately calls back into Python's own hash() of getId() rather than exposing
        // AGRC::Colour::hash()'s raw std::hash<std::string> value directly: the pure-Python
        // original's __hash__ was 'return hash(self.getId())' (Python's own string hash), which
        // this reproduces exactly rather than a numerically-different (but equally valid as a hash
        // function) C++ value.
        .def("__hash__", [](const AGRC::Colour &self) {
            return py::hash(py::cast(self.getId()));
        })

        .def("fromTuple", [](AGRC::Colour &self, const std::tuple<int, int, int, int> &colourTuple) {
            self.fromTuple(colourTuple);
        }, py::arg("colourTuple"), py::doc(R"doc(
Updates the colour based off 'colourTuple'

Parameters
----------
colourTuple: Tuple[:class:`int`, :class:`int`, :class:`int`, :class:`int`]
    The raw values for the colour in RGBA format
        )doc"))

        .def("getTuple", &AGRC::Colour::getTuple, py::doc(R"doc(
Retrieves the tuple representation of the colour in RGBA format

Returns
-------
Tuple[:class:`int`, :class:`int`, :class:`int`, :class:`int`]
    The colour tuple containing the following colour channel values, in order:

    #. Red
    #. Green
    #. Blue
    #. Alpha
        )doc"))

        .def("getId", &AGRC::Colour::getId, py::doc(R"doc(
Retrieves a unique id for the colour

.. note::
    The id generated will not correspond to any id generated from a colour range

Returns
-------
:class:`str`
    The id for the colour
        )doc"))

        .def("copy", &AGRC::Colour::copy, py::arg("colour"), py::arg("withAlpha") = true, py::doc(R"doc(
Copies the colour value from 'colour'

Parameters
----------
colour: :class:`CppColour`
    The colour to copy from

withAlpha: :class:`bool`
    Whether to also copy the alpha channel. **Default**: ``True``
        )doc"))

        .def("match", &AGRC::Colour::match, py::arg("colour"), py::doc(R"doc(
Whether 'colour' matches this colour

Parameters
----------
colour: :class:`CppColour`
    The colour to check

Returns
-------
:class:`bool`
    Whether the colour matches this colour
        )doc"));
}
