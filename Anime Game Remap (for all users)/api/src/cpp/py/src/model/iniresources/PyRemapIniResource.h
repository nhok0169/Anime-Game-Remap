#ifndef AGRemapPyBind_PyRemapIniResource_H
#define AGRemapPyBind_PyRemapIniResource_H

#include <pybind11/pybind11.h>

void initCppRemapIniResourceMixin(pybind11::module_ &m);
void initCppRemapIniResource(pybind11::module_ &m);
void initCppRemapIniFixResource(pybind11::module_ &m);
// initCppRemapIniGroupedResource moved to PyRemapIniGroupedResource.h
void initCppRemapIniDownload(pybind11::module_ &m);

#endif
