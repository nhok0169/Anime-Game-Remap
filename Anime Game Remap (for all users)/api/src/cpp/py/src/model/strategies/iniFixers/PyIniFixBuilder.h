#ifndef AGRemapPyBind_PyIniFixBuilder_H
#define AGRemapPyBind_PyIniFixBuilder_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


/**
 * @brief
 @rst
 Registers ``IniFixBuilder`` and its opaque ``CppIniFixBuilderArgs`` lookup table
 @endrst
 *
 * @param m The module to register into
 */
/**
 * @brief
 @rst
 Wraps a `Python`_ callable as an :cpp:type:`AGRemapCore::IniFixBuilder::Factory` -- see
 :cpp:func:`parseFactoryFromPy` for the two rules it exists to keep in one place
 @endrst
 *
 * @param factory The Python callable, taking ``(parser, toModName, modTypeId)``
 *
 * @return The wrapped factory
 */
AGRemapCore::IniFixBuilder::Factory fixFactoryFromPy(pybind11::object factory);

void initCppIniFixBuilder(pybind11::module_ &m);

#endif
