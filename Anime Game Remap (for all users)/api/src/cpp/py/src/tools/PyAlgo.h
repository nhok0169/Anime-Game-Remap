#ifndef AGRemapPyBind_PyAlgo_H
#define AGRemapPyBind_PyAlgo_H

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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/tools/Algo.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


class PyAlgo: public AGRC::Algo {
    public:
        static py::list pyMerge(const std::vector<std::vector<py::object>>& sortedLsts, const py::function& compare);
        static std::pair<bool, std::size_t> pyBinarySearch(const std::vector<py::object>& lst, const py::object& target, const py::function& compare);
};

void initCppAlgo(pybind11::module_ &m);

#endif