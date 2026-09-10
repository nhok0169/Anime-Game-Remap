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
