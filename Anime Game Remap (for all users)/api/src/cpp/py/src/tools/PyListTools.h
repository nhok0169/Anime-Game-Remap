#ifndef PyListTools_H
#define PyListTools_H

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