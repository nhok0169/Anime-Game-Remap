#ifndef PyTools_H
#define PyTools_H

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