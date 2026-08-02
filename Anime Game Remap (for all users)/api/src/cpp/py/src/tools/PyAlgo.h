#ifndef AGRemapPyBind_PyAlgo_H
#define AGRemapPyBind_PyAlgo_H

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