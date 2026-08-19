#ifndef AGRemapPyBind_PyIniClassifyStats_H
#define AGRemapPyBind_PyIniClassifyStats_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"


void initCppIniClassifyStats(pybind11::module_ &m);

#endif
