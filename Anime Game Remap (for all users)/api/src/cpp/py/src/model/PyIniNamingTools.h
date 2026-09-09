#ifndef AGRemapPyBind_PyIniNamingTools_H
#define AGRemapPyBind_PyIniNamingTools_H

#include <pybind11/pybind11.h>


/**
 * @brief
 @rst
 Binds `AGRC::IniNamingTools` as ``CppIniNamingTools`` -- deliberately NOT as ``IniNamingTools``,
 which the still-pure-`Python`_ class of that name already occupies and disagrees with. See the
 warning in the binding itself
 @endrst
 */
void initCppIniNamingTools(pybind11::module_ &m);

#endif
