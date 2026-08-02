#ifndef AGRemapPyBind_PyRanges_H
#define AGRemapPyBind_PyRanges_H
 
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
 
#include "AGRemapCore/tools/Ranges.h"
 
 
namespace py = pybind11;
namespace AGRC = AGRemapCore; 
 
 
/**
 * @brief
 @rst
 The `pybind11`_ bound version of `Ranges`
 @endrst
 */
class PyRanges: public AGRC::Ranges {
    public:
        using AGRC::Ranges::Ranges;
};
 
 
/**
 * @brief
 @rst
 The `pybind11`_ trampoline class for `PyRanges`, allowing `PyRanges` to be subclassed from `Python`_
 @endrst
 */
class PyBindRanges: public PyRanges {
    public:
        using PyRanges::PyRanges;
 
        bool has(int value) const override;
        bool isEmpty() const override;
        bool isFull() const override;
};
 
 
void initCppRanges(pybind11::module_ &m);

#endif
 