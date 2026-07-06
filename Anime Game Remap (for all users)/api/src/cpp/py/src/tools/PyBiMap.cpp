#include "PyBiMap.h"


template class AGRC::BiMap<py::object, py::object, PyObjectHash, PyObjectEqual, PyObjectHash, PyObjectEqual>;

void initCppBiMap(pybind11::module_ &m) {
    py::class_<CppBiMap>(m, "BiMap", 
        R"doc(
        A one-to-one dictionary
        )doc")

        .def(py::init<>())
        
        .def("__len__", &CppBiMap::size)

        .def("clear", &CppBiMap::clear)

        .def("empty", &CppBiMap::empty)

        .def("add", &CppBiMap::add, py::arg("key"), py::arg("val"))

        .def("insert", &CppBiMap::insert, py::arg("key"), py::arg("val"))

        .def("getKey",
            static_cast<const py::object& (CppBiMap::*)(const py::object&) const>(&CppBiMap::getKey),
            py::arg("val"))

        .def("getValue",
            static_cast<const py::object& (CppBiMap::*)(const py::object&) const>(&CppBiMap::getValue),
            py::arg("key"))

        .def("findKey",
            static_cast<std::optional<py::object> (CppBiMap::*)(const py::object&) const>(&CppBiMap::findKey),
            py::arg("val"))

        .def("findValue",
            static_cast<std::optional<py::object> (CppBiMap::*)(const py::object&) const>(&CppBiMap::findValue),
            py::arg("key"));
}