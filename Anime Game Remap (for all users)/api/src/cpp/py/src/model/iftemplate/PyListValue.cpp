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

#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/iftemplate/ListValue.h"
#include "AGRemapCore/model/IniSectionText.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppListValue(pybind11::module_ &m) {
    py::class_<AGRC::ListValue>(m, "ListValue", R"doc(
The value of a `KVP`_ read as a list of fields: ``a``, ``a, b`` or ``a, b, c, d, e, f, g, h`` -- whatever length the
value has

A value with no separator is a list of one field, so every `KVP`_ is one: ``vb0 = ResourceX`` and
``drawindexed = 12, 0, 0`` are read the same way. An empty value has no fields, and a trailing separator gives an
empty last field (``a, b,`` is three fields). Each field is held without its surrounding whitespace

A field is addressed by its position, counting from ``0``; a negative position counts from the end, so ``-1`` is
the last field whatever the length. Reading a field past either end gives ``None`` rather than an error

:meth:`str` writes the value back: exactly as it was given while no field has been set, and otherwise its fields
joined with the separator and a space

.. code-block:: python

    ListValue("12, 30, 0").number(1)    # 30
    ListValue("auto").number(0)         # None
    ListValue("0").size()               # 1

Parameters
----------
value: :class:`str`
    The value of the `KVP`_ :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

separator: :class:`str`
    The character the fields are separated by :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``","``
    )doc")
        .def(py::init<std::string, char>(), py::arg("value") = "", py::arg("separator") = ',')
        .def("size", &AGRC::ListValue::size, "How many fields the value has -- ``0`` for an empty value")
        .def("__len__", &AGRC::ListValue::size)
        .def_property_readonly("fields", &AGRC::ListValue::fields,
                               ":class:`list` [:class:`str`]: Every field, in order, without its surrounding whitespace")
        .def("at", &AGRC::ListValue::at, py::arg("pos"), py::doc(R"doc(
The field at 'pos', counting from the end when 'pos' is negative

Parameters
----------
pos: :class:`int`
    The field's position

Returns
-------
Optional[:class:`str`]
    The field, or ``None`` past either end
        )doc"))
        .def("number", &AGRC::ListValue::number, py::arg("pos"), py::doc(R"doc(
The field at 'pos' as a whole number -- an optional ``-`` followed by digits only

Parameters
----------
pos: :class:`int`
    The field's position, counting from the end when negative

Returns
-------
Optional[:class:`int`]
    The number, or ``None`` for any other field (``auto``, ``$var``, ``1.5``, an empty field) or a position past either
    end
        )doc"))
        .def("numbers", &AGRC::ListValue::numbers, py::doc(R"doc(
Every field as a whole number (see :meth:`number`)

Returns
-------
Optional[List[:class:`int`]]
    The numbers, or ``None`` when the value is empty or any one of its fields is not one
        )doc"))
        .def("set", &AGRC::ListValue::set, py::arg("pos"), py::arg("field"), py::doc(R"doc(
Sets the field at 'pos', counting from the end when 'pos' is negative

Parameters
----------
pos: :class:`int`
    The field's position

field: :class:`str`
    The new field, held without its surrounding whitespace

Returns
-------
:class:`bool`
    Whether 'pos' named a field: a position past either end changes nothing
        )doc"))
        .def("setNumber", &AGRC::ListValue::setNumber, py::arg("pos"), py::arg("number"), py::doc(R"doc(
Sets the field at 'pos' to a whole number -- see :meth:`set`

Parameters
----------
pos: :class:`int`
    The field's position

number: :class:`int`
    The new field

Returns
-------
:class:`bool`
    Whether 'pos' named a field
        )doc"))
        .def("append", &AGRC::ListValue::append, py::arg("field"), "Adds a field at the end, held without its surrounding whitespace")
        .def("changed", &AGRC::ListValue::changed, "Whether any field has been set or added since the value was read")
        .def("str", &AGRC::ListValue::str, py::doc(R"doc(
The value: as it was given when nothing has changed, otherwise every field joined with the separator and a space
(``a, b, c``)

Returns
-------
:class:`str`
    The value
        )doc"))
        .def("__str__", &AGRC::ListValue::str);
}


void initCppSectionText(pybind11::module_ &m) {
    py::class_<AGRC::SectionText>(m, "SectionText", R"doc(
One `section`_ built from its lines rather than written as text: runs of ``key = value`` lines, ``if`` blocks and a
comment above the header, built into an :class:`IfTemplate` by :meth:`build`

Every method that adds to the section returns the builder, so the lines read in order:

.. code-block:: python

    section = (SectionText(z3, "TextureOverrideBody")
               .keys([("hash", "abcd1234"), ("match_first_index", "0")])
               .open("$mod_enabled")
               .key("handling", "skip")
               .close()
               .build())

Parameters
----------
z3Ctx: :class:`Z3Context`
    The context the section's ``if`` conditions are parsed in -- kept alive with the builder

name: :class:`str`
    The section's name, without its brackets
    )doc")
        .def(py::init<AGRC::Z3Context&, std::string>(), py::arg("z3Ctx"), py::arg("name"), py::keep_alive<1, 2>())
        .def("keys", &AGRC::SectionText::keys, py::arg("kvps"), py::return_value_policy::reference_internal, py::doc(R"doc(
Adds a run of ``key = value`` lines. A value of ``""`` is a line with no ``=`` at all -- 3dmigoto's ``local $var``

Parameters
----------
kvps: List[Tuple[:class:`str`, :class:`str`]]
    The lines, in order

Returns
-------
:class:`SectionText`
    This builder
        )doc"))
        .def("key", &AGRC::SectionText::key, py::arg("key"), py::arg("value") = "", py::return_value_policy::reference_internal,
             "Adds one ``key = value`` line -- see :meth:`keys`. Returns this builder")
        .def("open", &AGRC::SectionText::open, py::arg("predicate"), py::return_value_policy::reference_internal,
             "Opens an ``if <predicate>`` block: the lines added after it sit inside it until :meth:`close`. Returns this builder")
        .def("orElse", &AGRC::SectionText::orElse, py::return_value_policy::reference_internal,
             "Adds the ``else`` of the innermost open ``if``. Returns this builder")
        .def("close", &AGRC::SectionText::close, py::return_value_policy::reference_internal,
             "Closes the innermost open ``if`` with an ``endif``. Returns this builder")
        .def("prefix", &AGRC::SectionText::prefix, py::arg("text"), py::return_value_policy::reference_internal,
             "Sets the comment written above the ``[name]`` line. Returns this builder")
        .def("build", &AGRC::SectionText::build, py::doc(R"doc(
The section built so far. Its lines move into the result, so a builder is built once

Returns
-------
:class:`IfTemplate`
    The section
        )doc"))
        .def("str", &AGRC::SectionText::str, py::doc(R"doc(
The section built so far, rendered as .ini text with a blank line after it. Its lines move into the result, as with
:meth:`build`

Returns
-------
:class:`str`
    The text
        )doc"));
}
