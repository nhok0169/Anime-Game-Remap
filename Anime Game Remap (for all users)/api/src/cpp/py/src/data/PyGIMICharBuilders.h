#ifndef AGRemapPyBind_PyGIMICharBuilders_H
#define AGRemapPyBind_PyGIMICharBuilders_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 A built :cpp:type:`AGRemapCore::IniParseBuilder::Factory`, wrapped so it can be handed straight to
 ``CppStrategyOverrides.setParser`` :raw-html:`<br />` :raw-html:`<br />`

 The alternative -- a `Python`_ callable that returns a strategy -- would have a C++-constructed
 parser cross into `Python`_ and back out through ``holdPyStrategy``, for no reason: nothing in
 `Python`_ ever touches it. ``setParser`` recognises this type and uses the factory as-is
 @endrst
 */
struct PyIniParseFactory {
    AGRC::IniParseBuilder::Factory factory;
};


/**
 * @brief The fixer counterpart of `PyIniParseFactory`
 */
struct PyIniFixFactory {
    AGRC::IniFixBuilder::Factory factory;
};


void initCppGIMICharBuilders(pybind11::module_ &m);

#endif
