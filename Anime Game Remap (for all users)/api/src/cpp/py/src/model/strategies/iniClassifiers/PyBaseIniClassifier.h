#ifndef AGRemapPyBind_PyBaseIniClassifier_H
#define AGRemapPyBind_PyBaseIniClassifier_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"


void initCppBaseIniClassifier(pybind11::module_ &m);

#endif
