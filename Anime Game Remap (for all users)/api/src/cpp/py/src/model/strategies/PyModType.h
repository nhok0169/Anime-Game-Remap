#ifndef AGRemapPyBind_PyModType_H
#define AGRemapPyBind_PyModType_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/ModType.h"


void initCppModType(pybind11::module_ &m);

#endif
