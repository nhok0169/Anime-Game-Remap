#ifndef AGRemapPyBind_PyBufElementType_H
#define AGRemapPyBind_PyBufElementType_H

#include <pybind11/pybind11.h>

#include <memory>
#include <vector>

#include "AGRemapCore/model/buffers/BufDataType.h"

/**
 * @brief Takes ownership of every data type in 'dataTypes' (each already an existing
 *      CppBufDataType-derived Python object) -- same ownership-transfer contract as
 *      PyIfTemplate.cpp's own 'parts' parsing (see Testing/CLAUDE.md's note on the resulting
 *      "Python instance was disowned" behaviour for the original objects)
 *
 * @param dataTypes The Python iterable of CppBufDataType-derived instances
 *
 * @return The data types, now owned by the returned vector
 */
std::vector<std::unique_ptr<AGRemapCore::BufDataType>> parseBufDataTypes(const pybind11::object &dataTypes);

void initCppBufElementType(pybind11::module_ &m);

#endif
