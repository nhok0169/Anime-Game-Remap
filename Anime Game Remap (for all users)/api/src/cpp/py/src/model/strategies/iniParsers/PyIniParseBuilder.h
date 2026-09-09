#ifndef AGRemapPyBind_PyIniParseBuilder_H
#define AGRemapPyBind_PyIniParseBuilder_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


/**
 * @brief
 @rst
 Registers ``IniParseBuilder`` and its opaque ``CppIniParseBuilderArgs`` lookup table
 @endrst
 *
 * @param m The module to register into
 */
/**
 * @brief
 @rst
 Wraps a `Python`_ callable as an :cpp:type:`AGRemapCore::IniParseBuilder::Factory` :raw-html:`<br />`
 :raw-html:`<br />`

 Takes the GIL (the core calls a factory from plain C++, where it is not held) and returns the
 result through ``holdPyStrategy``, which is what keeps a `Python`_ subclass's identity instead of
 slicing it away. Shared with :cpp:class:`AGRemapCore::StrategyOverrides`' binding so there is one
 implementation of both of those rules
 @endrst
 *
 * @param factory The Python callable, taking ``(iniFile, modTypeId)``
 *
 * @return The wrapped factory
 */
AGRemapCore::IniParseBuilder::Factory parseFactoryFromPy(pybind11::object factory);

void initCppIniParseBuilder(pybind11::module_ &m);

#endif
