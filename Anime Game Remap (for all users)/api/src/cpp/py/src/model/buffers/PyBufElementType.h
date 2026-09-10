#ifndef AGRemapPyBind_PyBufElementType_H
#define AGRemapPyBind_PyBufElementType_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

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
