#ifndef AGRemapPyBind_PyIniFixBuilder_H
#define AGRemapPyBind_PyIniFixBuilder_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


/**
 * @brief
 @rst
 Registers ``CppIniFixBuilder`` and its opaque ``CppIniFixBuilderArgs`` lookup table
 @endrst
 *
 * @param m The module to register into
 */
void initCppIniFixBuilder(pybind11::module_ &m);

#endif
