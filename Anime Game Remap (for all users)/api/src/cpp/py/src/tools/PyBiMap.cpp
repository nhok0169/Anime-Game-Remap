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