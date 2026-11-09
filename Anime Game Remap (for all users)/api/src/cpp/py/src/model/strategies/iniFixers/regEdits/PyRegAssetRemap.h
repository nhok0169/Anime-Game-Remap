#ifndef AGRemapPyBind_PyRegAssetRemap_H
#define AGRemapPyBind_PyRegAssetRemap_H

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

#include <optional>
#include <string>

#include <pybind11/pybind11.h>

#include "PyBaseRegEdit.h"
#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `Python`_ ``RegAssetRemap`` is :cpp:class:`AGRemapCore::RegAssetRemap` itself: its arguments are parsed into the C++ members
 when it is built or they are set, and read back from them -- so one built in `Python`_ runs in a C++ fixer's list, and
 one built in C++ reads in `Python`_ as a ``RegAssetRemap``
 @endrst
 */
using PyRegAssetRemap = AGRC::RegAssetRemap<std::string, std::string>;


void initCppRegAssetRemap(pybind11::module_ &m);

#endif
