#ifndef AGRemapPyBind_PyIniClassifier_H
#define AGRemapPyBind_PyIniClassifier_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifier.h"


void initCppIniClassifier(pybind11::module_ &m);

#endif
