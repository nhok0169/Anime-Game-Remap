#ifndef AGRemapPyBind_PyIniResource_H
#define AGRemapPyBind_PyIniResource_H

#include <pybind11/pybind11.h>

void initCppIniResource(pybind11::module_ &m);
void initCppIniFixResource(pybind11::module_ &m);
// initCppIniGroupedResource moved to PyIniGroupedResource.h

#endif
