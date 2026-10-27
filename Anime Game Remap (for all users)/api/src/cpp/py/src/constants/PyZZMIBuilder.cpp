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

#include "PyZZMIBuilder.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppZZMIBuilder(pybind11::module_ &m) {
    py::class_<AGRC::ZZMIBuilder>(m, "ZZMIBuilder", R"doc(
Creates new :class:`ModType` objects for ZZZ (Zenless Zone Zero) mods -- the ZZMI counterpart of
:class:`GIBuilder`. A ZZMI mod is GIMI-shaped, and every component of a ZZZ character skins from its
own bone palette
    )doc")
        .def_static("miyabi", &AGRC::ZZMIBuilder::miyabi, py::doc(R"doc(Creates the :class:`ModType` for Miyabi)doc"))
        .def_static("miyabiBlossom", &AGRC::ZZMIBuilder::miyabiBlossom, py::doc(R"doc(Creates the :class:`ModType` for MiyabiBlossom (Miyabi's Dignified Blossom skin))doc"))
        .def_static("all", &AGRC::ZZMIBuilder::all, py::doc(R"doc(Every ZZZ :class:`ModType`, freshly built on each call)doc"));
}
