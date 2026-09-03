#ifndef AGRemapPyBind_PyIniParseBuilder_H
#define AGRemapPyBind_PyIniParseBuilder_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


/**
 * @brief
 @rst
 Registers ``CppIniParseBuilder`` and its opaque ``CppIniParseBuilderArgs`` lookup table
 @endrst
 *
 * @param m The module to register into
 */
void initCppIniParseBuilder(pybind11::module_ &m);

#endif
