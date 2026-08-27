#include "PyIniResourceModel.h"

#include "AGRemapCore/model/iniresources/IniResourceModel.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIniResourceModel(pybind11::module_ &m) {
    py::class_<AGRC::IniResourceModel>(m, "IniResourceModel", R"doc(
Contains data for some particular resource in a .ini file
    )doc")

        .def(py::init<std::string>(), py::arg("iniFolderPath"), py::doc(R"doc(
Constructs new data for a resource in a .ini file

Parameters
----------
iniFolderPath: :class:`str`
    The folder path to where the .ini file of the resource is located
        )doc"))

        .def_readwrite("iniFolderPath", &AGRC::IniResourceModel::iniFolderPath, py::doc(R"doc(
:class:`str`: The folder path to where the .ini file of the resource is located
        )doc"));
}
