#ifndef AGRemapPyBind_PyTools_H
#define AGRemapPyBind_PyTools_H

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

namespace py = pybind11;


// PyObjectLess: Eq comparison using python's builtin __eq__ function
struct PyObjectEqual {
    bool operator()(const py::object& lhs, const py::object& rhs) const {
        return lhs.equal(rhs);
    }
};

// PyObjectHash: python's builtin __hash__ function
struct PyObjectHash {
    std::size_t operator()(const py::object& obj) const {
        return py::hash(obj);
    }
};

#endif