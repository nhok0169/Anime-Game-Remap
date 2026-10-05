#ifndef AGRemapPyBind_PyListTools_H
#define AGRemapPyBind_PyListTools_H

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

#include <unordered_set>
#include <map>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/tools/ListTools.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore; 


class PyListTools: public AGRC::ListTools {
    public:
        static py::list removeParts(py::list lst, py::list partIndices);
        static py::list removeByInds(py::list lst, const std::unordered_set<std::size_t>& inds);
        static py::list addLstsByInds(py::list lst, const std::map<long long, py::list>& subLsts);
};

void initCppListTools(pybind11::module_ &m);

#endif