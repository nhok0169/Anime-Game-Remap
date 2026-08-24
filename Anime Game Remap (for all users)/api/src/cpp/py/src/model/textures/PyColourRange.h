#ifndef AGRemapPyBind_PyColourRange_H
#define AGRemapPyBind_PyColourRange_H

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
