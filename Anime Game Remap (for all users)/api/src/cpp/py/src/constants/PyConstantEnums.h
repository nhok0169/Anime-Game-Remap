#ifndef AGRemapPyBind_PyConstantEnums_H
#define AGRemapPyBind_PyConstantEnums_H

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

#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/constants/IniGraphReplaceMode.h"
#include "AGRemapCore/constants/RegFillMissingMode.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The registered `Python`_ member of a bound core enum -- the very object ``Enum.Member`` evaluates
 to :raw-html:`<br />` :raw-html:`<br />`

 ``py::cast(value)`` builds a NEW `Python`_ object for an enum each time, which compares equal to
 the member but is not it. A binding that hands back a default its caller never passed (eg.
 :class:`RegFillMissing`'s ``fillMode``) uses this instead, so ``edit.fillMode is
 RegFillMissingMode.FillMissing`` holds
 @endrst
 *
 * @tparam E The bound core enum
 * @param value The enum value to fetch the member for
 */
template<typename E>
py::object enumMember(E value) {
    // py::type::of<E>() does not compile for an enum (its caster is not the generic class caster),
    // so the type comes from the freshly cast member instead
    py::object member = py::cast(value);
    return py::type::of(member).attr(member.attr("name"));
}


/**
 * @brief
 @rst
 Reads a `Python`_ argument naming a :class:`DownloadMode` -- the bound member itself or, for a
 value read off the command line, its name (``"normal"``, ``"disabled"``, ``"always"``, matched
 by :cpp:func:`AGRC::DownloadModeTools::findByName`) :raw-html:`<br />` :raw-html:`<br />`

 ``None`` reads as :cpp:enumerator:`AGRC::DownloadMode::Normal`
 @endrst
 *
 * @param mode The `Python`_ argument
 * @param strict Whether a name that matches no mode raises ``ValueError``, rather than reading as
 *      :cpp:enumerator:`AGRC::DownloadMode::Normal`
 */
AGRC::DownloadMode toDownloadMode(const py::object &mode, bool strict = true);

/**
 * @brief
 @rst
 Reads a `Python`_ argument naming a :class:`RegFillMissingMode`: the bound member, or the
 lowercase-first name it used to carry as a string value (``"fillMissing"``, ``"topdownCover"``,
 ``"bottomCover"``). Anything else, ``None`` included, reads as
 :cpp:enumerator:`AGRC::RegFillMissingMode::FillMissing`
 @endrst
 *
 * @param mode The `Python`_ argument
 */
AGRC::RegFillMissingMode toRegFillMissingMode(const py::object &mode);

/**
 * @brief
 @rst
 Reads a `Python`_ argument naming an :class:`IniGraphReplaceMode`: the bound member, or the
 lowercase name it used to carry as a string value (``"ignore"``, ``"replace"``, ``"combine"``).
 Anything else, ``None`` included, reads as :cpp:enumerator:`AGRC::IniGraphReplaceMode::Ignore`
 @endrst
 *
 * @param mode The `Python`_ argument
 */
AGRC::IniGraphReplaceMode toIniGraphReplaceMode(const py::object &mode);

/**
 * @brief
 @rst
 Reads a `Python`_ argument naming an :class:`IfPredPartType`: the bound member, or its keyword
 (``"if"``, ``"else"``, ``"elif"``, ``"endif"``)
 @endrst
 *
 * @param type The `Python`_ argument
 *
 * @throws py::value_error If 'type' names no :class:`IfPredPartType`
 */
AGRC::IfPredPartType toIfPredPartType(const py::object &type);


void initCppConstantEnums(pybind11::module_ &m);

#endif
