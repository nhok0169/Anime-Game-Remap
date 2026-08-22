#include "PyZ3Predicate.h"

#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppZ3Predicate(pybind11::module_ &m) {
    py::class_<AGRC::Z3Predicate>(m, "Z3Predicate", R"doc(
An opaque, boolean-sorted `Z3`_ predicate -- produced by :meth:`IfPredPart.getLogicQuery`, and the
input :meth:`IfPredPart.getIfPredStr` expects

.. note::
    Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a
    real, independent copy

.. warning::
    ``&``/``|``/``~`` (and :meth:`isSatisfiable`, since it builds a solver over this predicate's
    own context) all require every operand to belong to the same :class:`Z3Context` (see
    :meth:`belongsTo`/:meth:`sameContext`) -- combining predicates from two different contexts is
    a `Z3`_-level precondition violation that is not guaranteed to raise a catchable error. Use
    :meth:`IfPredPart.reparent` to move a predicate into a different context first if it isn't
    already guaranteed to match
    )doc")
        .def("toString", &AGRC::Z3Predicate::toString,
    py::doc(R"doc(
The predicate rendered as a `Z3`_ SMT-LIB2 expression string

Returns
-------
:class:`str`
    The string form of the predicate
        )doc"))

        .def("__str__", &AGRC::Z3Predicate::toString)
        .def("__repr__", [](const AGRC::Z3Predicate &self) { return "Z3Predicate(" + self.toString() + ")"; })

        .def("__copy__", [](const AGRC::Z3Predicate &self) { return self; })
        .def("__deepcopy__", [](const AGRC::Z3Predicate &self, const py::dict &) { return self; })

        .def("__and__", &AGRC::Z3Predicate::operator&, py::arg("other"),
    py::doc(R"doc(
Logical AND with another predicate; supports the ``&`` operator

Parameters
----------
other: :class:`Z3Predicate`
    The predicate to combine with -- must belong to the same :class:`Z3Context` as this predicate
    (see :meth:`belongsTo`)

Returns
-------
:class:`Z3Predicate`
    The combined predicate, in this predicate's own context
        )doc"))

        .def("__or__", &AGRC::Z3Predicate::operator|, py::arg("other"),
    py::doc(R"doc(
Logical OR with another predicate; supports the ``|`` operator

Parameters
----------
other: :class:`Z3Predicate`
    The predicate to combine with -- must belong to the same :class:`Z3Context` as this predicate
    (see :meth:`belongsTo`)

Returns
-------
:class:`Z3Predicate`
    The combined predicate, in this predicate's own context
        )doc"))

        .def("__invert__", &AGRC::Z3Predicate::operator!,
    py::doc(R"doc(
Logical negation of this predicate; supports the ``~`` operator

Returns
-------
:class:`Z3Predicate`
    The negated predicate, in this predicate's own context
        )doc"))

        .def("simplify", &AGRC::Z3Predicate::simplify,
    py::doc(R"doc(
A simplified, logically-equivalent form of this predicate

Returns
-------
:class:`Z3Predicate`
    The simplified predicate
        )doc"))

        .def("isSatisfiable", &AGRC::Z3Predicate::isSatisfiable,
    py::doc(R"doc(
Whether this predicate is satisfiable -- ie. whether some assignment of its free variables makes
it evaluate to ``True``, checked via a real `Z3`_ solver

Returns
-------
:class:`bool`
    Whether this predicate is satisfiable
        )doc"))

        .def("sameContext", &AGRC::Z3Predicate::sameContext, py::arg("other"),
    py::doc(R"doc(
Whether 'other' belongs to the same :class:`Z3Context` as this predicate (a plain identity check,
not a check of logical equivalence)

Parameters
----------
other: :class:`Z3Predicate`
    The predicate to compare against

Returns
-------
:class:`bool`
    Whether both predicates share the same underlying `Z3`_ context
        )doc"))

        .def("belongsTo", &AGRC::Z3Predicate::belongsTo, py::arg("ctx"),
    py::doc(R"doc(
Whether this predicate belongs to 'ctx' (a plain identity check, not a check of logical equivalence)

Parameters
----------
ctx: :class:`Z3Context`
    The context to compare against

Returns
-------
:class:`bool`
    Whether this predicate belongs to 'ctx'
        )doc"))

        .def_static("trueValue", &AGRC::Z3Predicate::trueValue, py::arg("ctx"),
    py::doc(R"doc(
The literal ``True`` predicate, in the given `Z3`_ context

Parameters
----------
ctx: :class:`Z3Context`
    The context the returned predicate will belong to

Returns
-------
:class:`Z3Predicate`
    The literal ``True`` predicate
        )doc"))

        .def_static("falseValue", &AGRC::Z3Predicate::falseValue, py::arg("ctx"),
    py::doc(R"doc(
The literal ``False`` predicate, in the given `Z3`_ context

Parameters
----------
ctx: :class:`Z3Context`
    The context the returned predicate will belong to

Returns
-------
:class:`Z3Predicate`
    The literal ``False`` predicate
        )doc"));
}
