#ifndef AGRemapPyBind_PyIniRemovalContext_H
#define AGRemapPyBind_PyIniRemovalContext_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniRemovers/IniRemovalContext.h"


/**
 * @brief
 @rst
 Registers :cpp:class:`AGRemapCore::IniRemovalContext` with `Python`_ :raw-html:`<br />`
 :raw-html:`<br />`

 Bound straight, with no ``Py``-prefixed subclass in between: the struct is not a template, holds no
 `Python`_ state, and has nothing a `Python`_ caller could want that it does not already have
 @endrst
 *
 * @param m The module to register into
 */
void initCppIniRemovalContext(pybind11::module_ &m);

#endif
