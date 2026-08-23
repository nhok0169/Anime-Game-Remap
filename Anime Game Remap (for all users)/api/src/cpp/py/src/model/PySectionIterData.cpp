#include "PySectionIterData.h"

#include "iftemplate/PyIfContentPartColour.h"
#include "../tools/z3/PyZ3Predicate.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppSectionIterData(pybind11::module_ &m) {
    py::class_<PySectionIterData>(m, "SectionIterData", R"doc(
A class that contains the needed data for each iteration after calling :meth:`IniSectionGraph.iterSectsByContentPart`

Parameters
----------
sectionName: :class:`str`
    The name of the `section`_

section: :class:`IfTemplate`
    The corresponding `section`_ the part resides in

part: :class:`IfContentPart`
    The corresponding part

state: :class:`int`
    The current state of the `section`_

colouring: Optional[:class:`IfContentPartColouring`]
    The current `KVP`_ states of the :class:`IfContentPart`
    )doc")

        .def_readonly("sectionName", &PySectionIterData::sectionName, py::doc(R"doc(:class:`str`: The name of the `section`_)doc"))

        .def_property_readonly("section", [](PySectionIterData &self) -> PyIfTemplate* {
            return self.section;
        }, py::return_value_policy::reference, py::doc(R"doc(:class:`IfTemplate`: The corresponding `section`_ the part resides in)doc"))

        .def_property_readonly("part", [](PySectionIterData &self) -> PyIfContentPart* {
            return self.part;
        }, py::return_value_policy::reference, py::doc(R"doc(:class:`IfContentPart`: The corresponding part)doc"))

        .def_readonly("state", &PySectionIterData::state, py::doc(R"doc(:class:`int`: The current state of the `section`_)doc"))

        .def_property_readonly("colouring", [](PySectionIterData &self) -> py::object {
            if (self.colouring == nullptr) return py::none();
            return py::cast(self.colouring, py::return_value_policy::reference);
        }, py::doc(R"doc(Optional[:class:`IfContentPartColouring`]: The current `KVP`_ states of the :class:`IfContentPart`)doc"));

    py::class_<PySectionIterQueryData>(m, "SectionIterQueryData", R"doc(
A class that contains the needed data for each iteration after calling :meth:`IniSectionGraph.iterByQuery`

Parameters
----------
part: :class:`IfContentPart`
    The part retrieved

query: :class:`Z3Predicate`
    The corresponding logical query that the part resides in

sectionName: :class:`str`
    The name of the `section`_ the part resides in

section: :class:`IfTemplate`
    The corresponding `section`_ the part resides in

rootSectionName: :class:`str`
    The name of the root `section`_ the part resides in

rootSection: :class:`IfTemplate`
    The corresponding root `section`_ the part resides in

state: :class:`int`
    The current state the `section`_ is in

colouring: Optional[:class:`IfContentPartColouring`]
    The current `KVP`_ states of the :class:`IfContentPart`
    )doc")

        .def_property_readonly("part", [](PySectionIterQueryData &self) -> PyIfContentPart* {
            return self.part;
        }, py::return_value_policy::reference, py::doc(R"doc(:class:`IfContentPart`: The part retrieved)doc"))

        .def_readonly("query", &PySectionIterQueryData::query, py::doc(R"doc(:class:`Z3Predicate`: The corresponding logical query that the part resides in)doc"))

        .def_readonly("sectionName", &PySectionIterQueryData::sectionName, py::doc(R"doc(:class:`str`: The name of the `section`_ the part resides in)doc"))

        .def_property_readonly("section", [](PySectionIterQueryData &self) -> PyIfTemplate* {
            return self.section;
        }, py::return_value_policy::reference, py::doc(R"doc(:class:`IfTemplate`: The corresponding `section`_ the part resides in)doc"))

        .def_readonly("rootSectionName", &PySectionIterQueryData::rootSectionName, py::doc(R"doc(:class:`str`: The name of the root `section`_ the part resides in)doc"))

        .def_property_readonly("rootSection", [](PySectionIterQueryData &self) -> PyIfTemplate* {
            return self.rootSection;
        }, py::return_value_policy::reference, py::doc(R"doc(:class:`IfTemplate`: The corresponding root `section`_ the part resides in)doc"))

        .def_readonly("state", &PySectionIterQueryData::state, py::doc(R"doc(:class:`int`: The current state the `section`_ is in)doc"))

        .def_property_readonly("colouring", [](PySectionIterQueryData &self) -> py::object {
            if (self.colouring == nullptr) return py::none();
            return py::cast(self.colouring, py::return_value_policy::reference);
        }, py::doc(R"doc(Optional[:class:`IfContentPartColouring`]: The current `KVP`_ states of the :class:`IfContentPart`)doc"));
}
