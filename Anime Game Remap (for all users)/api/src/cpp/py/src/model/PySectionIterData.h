#ifndef AGRemapPyBind_PySectionIterData_H
#define AGRemapPyBind_PySectionIterData_H

#include <pybind11/pybind11.h>

#include "iftemplate/PyIfContentPart.h"  // reuses PyIfContentPart/PyObjectHash/PyObjectEqual
#include "iftemplate/PyIfTemplate.h"
#include "AGRemapCore/model/SectionIterData.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief The `pybind11`_-facing names for `AGRC::SectionIterData`/`AGRC::SectionIterQueryData`\\<py::object, py::object\\>. Plain aliases, not subclasses.
 */
using PySectionIterData = AGRC::SectionIterData<std::string, std::string>;
using PySectionIterQueryData = AGRC::SectionIterQueryData<std::string, std::string>;


void initCppSectionIterData(pybind11::module_ &m);

#endif
