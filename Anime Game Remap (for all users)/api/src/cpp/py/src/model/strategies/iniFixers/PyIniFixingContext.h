#ifndef AGRemapPyBind_PyIniFixingContext_H
#define AGRemapPyBind_PyIniFixingContext_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniFixers/IniFixingContext.h"


/**
 * @brief
 @rst
 Registers :cpp:class:`AGRemapCore::IniFixingContext` with `Python`_ :raw-html:`<br />`
 :raw-html:`<br />`

 Bound straight, with no ``Py``-prefixed subclass in between, for the same reasons
 ``PyIniRemovalContext`` is: the struct is not a template, holds no `Python`_ state, and has nothing
 a `Python`_ caller could want that it does not already have
 @endrst
 *
 * @param m The module to register into
 */
void initCppIniFixingContext(pybind11::module_ &m);

#endif
