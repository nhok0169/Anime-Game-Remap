#ifndef AGRemapPyBind_PyIniRemoveBuilder_H
#define AGRemapPyBind_PyIniRemoveBuilder_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"


/**
 * @brief
 @rst
 Registers ``CppIniRemoveBuilder`` and its opaque ``CppIniRemoveBuilderArgs`` lookup table
 @endrst
 *
 * @param m The module to register into
 */
void initCppIniRemoveBuilder(pybind11::module_ &m);

#endif
