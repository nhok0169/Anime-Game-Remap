#ifndef AGRemapPyBind_PyStrategyOverrides_H
#define AGRemapPyBind_PyStrategyOverrides_H

#include <pybind11/pybind11.h>


/**
 * @brief
 @rst
 Registers ``StrategyOverrides``, the runtime parser/fixer override table
 @endrst
 *
 * @param m The module to register into
 */
void initCppStrategyOverrides(pybind11::module_ &m);

#endif
