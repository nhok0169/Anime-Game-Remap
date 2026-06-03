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

        .def("insert", &CppBiMap::insert, py::arg("key"), py::arg("val"))

        .def("getKey", &CppBiMap::getKey, py::arg("val"))

        .def("getValue", &CppBiMap::getValue, py::arg("key"))

        .def("findKey", &CppBiMap::findValue, py::arg("key"))

        .def("findValue", &CppBiMap::findKey, py::arg("val"));
}