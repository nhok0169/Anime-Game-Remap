#ifndef PyBiMap_H
#define PyBiMap_H

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/tools/BiMap.h"
#include "PyTools.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


extern template class AGRC::BiMap<py::object, py::object, PyObjectHash, PyObjectEqual, PyObjectHash, PyObjectEqual>;

class CppBiMap: public AGRC::BiMap<py::object, py::object, PyObjectHash, PyObjectEqual, PyObjectHash, PyObjectEqual> {

};


void initCppBiMap(pybind11::module_ &m);

#endif