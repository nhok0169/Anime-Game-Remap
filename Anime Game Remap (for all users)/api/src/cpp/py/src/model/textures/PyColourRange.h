#ifndef AGRemapPyBind_PyColourRange_H
#define AGRemapPyBind_PyColourRange_H

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

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/textures/ColourRange.h"

/**
 * @brief Parses a Python `Optional[Set[Union[Colour, ColourRange]]]`-shaped object into a
 *      AGRemapCore::ColourOrRangeSet -- shared by every binding that accepts this parameter shape
 *      (CppColourReplace, CppColourReplaceFilter, CppTransparencyAdjustFilter, ...)
 *
 * @param obj A Python `None`, or an iterable of CppColour/CppColourRange instances
 *
 * @return `std::nullopt` if 'obj' is `None`, otherwise the parsed set
 */
std::optional<AGRemapCore::ColourOrRangeSet> parseColourOrRangeSet(const pybind11::object &obj);

/**
 * @brief The inverse of parseColourOrRangeSet -- converts back to a Python `set`, or `None`
 *
 * @param colourOrRangeSet The set to convert
 */
pybind11::object colourOrRangeSetToPy(const std::optional<AGRemapCore::ColourOrRangeSet> &colourOrRangeSet);

void initCppColourRange(pybind11::module_ &m);

#endif
