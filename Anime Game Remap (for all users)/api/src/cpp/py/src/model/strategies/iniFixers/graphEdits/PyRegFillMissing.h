#ifndef AGRemapPyBind_PyRegFillMissing_H
#define AGRemapPyBind_PyRegFillMissing_H

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

#include "PyBaseIniGraphEdit.h"
#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/constants/RegFillMissingMode.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegFillMissing.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 Reads a `Python`_-side ``IniFile``'s own ``downloadMode`` and converts it into the core enum
 :raw-html:`<br />` :raw-html:`<br />`

 An ``ini`` that is ``None``, or that carries no ``downloadMode`` attribute at all, reads as
 ``DownloadMode::Normal`` -- the mode under which this edit behaves identically to
 ``dependOnDownload`` being ``False``. The pure-Python original raised ``AttributeError`` for that
 case instead; nothing depends on the crash, and a graph edit is routinely handed a stand-in ``ini``
 by ``GraphGroupEdit``'s own callers
 @endrst
 *
 * @param ini The Python .ini file to read the download mode off
 */
AGRC::DownloadMode parseIniDownloadMode(const py::object &ini);


/**
 * @brief
 @rst
 The `Python`_ ``RegFillMissing`` is :cpp:class:`AGRemapCore::RegFillMissing` itself: its arguments are parsed into the C++ members
 when it is built or they are set, and read back from them -- so one built in `Python`_ runs in a C++ fixer's list, and
 one built in C++ reads in `Python`_ as a ``RegFillMissing``
 @endrst
 */
using PyRegFillMissing = AGRC::RegFillMissing<std::string, std::string>;


/**
 * @brief
 @rst
 Builds the :cpp:type:`AGRemapCore::RegFillMissing::FillMissingFunc` one of ``fillMissing``'s three
 `Python`_ shapes means :raw-html:`<br />` :raw-html:`<br />`

 A ``str`` adds ``reg = fillMissing``; a ``list`` adds every `KVP`_ tuple it holds; anything else
 callable is used as the modification itself. ``None`` (or a non-callable of any other type) yields
 an empty function, which every mode treats as a no-op
 @endrst
 *
 * @param fillMissing The Python value to build from
 * @param reg The register to add, for the ``str`` shape
 * @param toFront Whether the `KVPs`_ are added to the front of the part instead of the back
 */
PyRegFillMissing::FillMissingFunc parseFillMissing(const py::object &fillMissing, const std::string &reg, bool toFront);


/**
 * @brief
 @rst
 Converts a `Python`_ ``keysToTrack`` argument into the core's own optional key set
 :raw-html:`<br />` :raw-html:`<br />`

 ``None`` becomes ``std::nullopt``, meaning **every** key is tracked -- the same convention
 :cpp:func:`AGRemapCore::IfContentPartColouring::updateColouring` and :cpp:class:`GraphGroupEdit`
 already use. Any other iterable is materialized into a real key set
 @endrst
 *
 * @param keysToTrack The Python value to convert
 */
std::optional<PyRegFillMissing::KeySet> parseKeysToTrack(const py::object &keysToTrack);


/**
 * @brief
 @rst
 Wraps a `Python`_ ``partFilter`` callable as the core
 :cpp:type:`AGRemapCore::BaseIniGraphEdit::PartFilter` :cpp:func:`AGRemapCore::RegFillMissing::edit`
 takes :raw-html:`<br />` :raw-html:`<br />`

 The `Python`_ callable is invoked as ``partFilter(iterData, modType, ini)`` with the *captured*
 `Python`_ ``modType``/``ini`` objects rather than the core's own (always-null) pointers -- the same
 shape, and the same reason, as :cpp:class:`GraphGroupEdit`'s own ``keyFilters``. ``None`` (or any
 non-callable) yields an empty function, which accepts every part
 @endrst
 *
 * @param partFilter The Python callable to wrap
 * @param modType The Python mod type to hand it
 * @param ini The Python .ini file to hand it
 */
PyRegFillMissing::PartFilter parsePartFilter(const py::object &partFilter, py::object modType, py::object ini);


/**
 * @brief Registers the Python-facing ``RegFillMissing``
 *
 * @param m The module to register into
 */
void initCppRegFillMissing(pybind11::module_ &m);

#endif
