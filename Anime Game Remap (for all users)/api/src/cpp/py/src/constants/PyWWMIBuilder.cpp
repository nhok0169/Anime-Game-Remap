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

#include "PyWWMIBuilder.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppWWMIBuilder(pybind11::module_ &m) {
    py::class_<AGRC::WWMIBuilder>(m, "WWMIBuilder", R"doc(
Creates new :class:`ModType` objects for WuWa (Wuthering Waves) mods -- the WWMI counterpart of
:class:`GIBuilder`. Their parse / fix / remove rows are stubs until the WWMI strategies exist
    )doc")
        .def_static("sanhua", &AGRC::WWMIBuilder::sanhua, py::doc(R"doc(Creates the :class:`ModType` for Sanhua)doc"))
        .def_static("sanhuaExorcist", &AGRC::WWMIBuilder::sanhuaExorcist, py::doc(R"doc(Creates the :class:`ModType` for SanhuaExorcist (WWMI-Assets' SanhuaSkin1))doc"))
        .def_static("chisa", &AGRC::WWMIBuilder::chisa, py::doc(R"doc(Creates the :class:`ModType` for Chisa)doc"))
        .def_static("chisaParfait", &AGRC::WWMIBuilder::chisaParfait, py::doc(R"doc(Creates the :class:`ModType` for ChisaParfait)doc"))
        .def_static("all", &AGRC::WWMIBuilder::all, py::doc(R"doc(Every WuWa :class:`ModType`, freshly built on each call)doc"));
}
