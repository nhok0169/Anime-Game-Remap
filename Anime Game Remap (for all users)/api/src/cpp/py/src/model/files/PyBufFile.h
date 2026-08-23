#ifndef AGRemapPyBind_PyBufFile_H
#define AGRemapPyBind_PyBufFile_H

#include <pybind11/pybind11.h>

#include <memory>
#include <vector>

#include "AGRemapCore/model/buffers/BufElementType.h"

/**
 * @brief Takes ownership of every element in 'elements' (each already an existing
 *      CppBufElementType Python object) -- same ownership-transfer contract as
 *      PyBufElementType.h's own parseBufDataTypes, one hierarchy level up
 *
 * @param elements The Python iterable of CppBufElementType instances
 *
 * @return The elements, now owned by the returned vector
 */
std::vector<std::unique_ptr<AGRemapCore::BufElementType>> parseBufElementTypes(const pybind11::object &elements);

void initCppBufFile(pybind11::module_ &m);

#endif
