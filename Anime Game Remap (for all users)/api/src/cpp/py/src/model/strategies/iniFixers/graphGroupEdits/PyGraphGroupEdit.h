#ifndef AGRemapPyBind_PyGraphGroupEdit_H
#define AGRemapPyBind_PyGraphGroupEdit_H

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

#include <functional>
#include <memory>
#include <string>

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `Python`_ ``GraphGroupEdit`` is :cpp:class:`AGRemapCore::GraphGroupEdit` itself. Its ``edits``, ``trackKeys``,
 ``keysToTrack`` and ``keyFilters`` are parsed into the C++ table when they are given and read back from it, so one built
 in `Python`_ runs in a C++ fixer's list, and one built in C++ (a step of a shipped fix) reads in `Python`_ with its
 table. An edit of the library's goes into the table as itself; any other (a `Python`_ subclass) through an adapter that
 calls its own ``editFromIni`` with the C++ part, graph, :class:`ModType` and :class:`IniFile`
 @endrst
 */
using PyGraphGroupEdit = AGRC::GraphGroupEdit<std::string, std::string>;


class PyIniGraphGroups;

/**
 * @brief Whether 'object' is of a class the library binds, as itself -- a `Python`_ subclass's type is not the registered one
 */
bool isBound(const py::handle &object);

/**
 * @brief A `Python`_ object kept alive by a C++ owner, let go of with the GIL held
 */
std::shared_ptr<void> holdPy(py::object object);

/**
 * @brief The `Python`_ graphs of a call's groups, and a graph a `Python`_ edit hands back adopted into them -- for the
 *        call's :cpp:class:`PyEditScope`
 */
std::function<py::object(const void *)> graphToPyOf(PyIniGraphGroups &groups);
std::function<void *(py::object)> adoptOf(PyIniGraphGroups &groups);


void initCppGraphGroupEdit(pybind11::module_ &m);

#endif
