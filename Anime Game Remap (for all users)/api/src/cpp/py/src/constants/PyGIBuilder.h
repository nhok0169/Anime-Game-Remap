#ifndef AGRemapPyBind_PyGIBuilder_H
#define AGRemapPyBind_PyGIBuilder_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/constants/GIBuilder.h"


void initCppGIBuilder(pybind11::module_ &m);

#endif
