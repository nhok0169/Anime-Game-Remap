#ifndef AGRemapPyBind_PyIterGraphGroupEdit_H
#define AGRemapPyBind_PyIterGraphGroupEdit_H

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

#include "PyBaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IterGraphGroupEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::IterGraphGroupEdit`\\<std::string, std::string\\>
 :raw-html:`<br />` :raw-html:`<br />`

 Holds the `Python`_ factory as given, and runs the edits it returns through their own `Python`_
 ``edit`` / ``editFromIni`` -- the same reason `PyBaseIniGraphGroupEdit`'s ``editFromIni`` goes
 through ``self.attr("edit")``: a returned edit may be a pure-Python subclass, or a bound one whose
 `Python`_ ``edit`` re-reads its `Python`_ attributes first (eg. `PyGraphRemove`), and a C++
 virtual call would reach neither
 @endrst
 */
class PyIterGraphGroupEdit: public AGRC::IterGraphGroupEdit<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::IterGraphGroupEdit<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``makeEdits`` -- a callable taking
         ``(iniIndex, graphGroups)`` and returning a list of graph-group edits, or ``None``
         @endrst
         */
        py::object makeEditsObj;

        /**
         * @brief Constructs a new edit run on every group
         *
         * @param makeEditsObj The Python factory that builds each group's edits
         */
        explicit PyIterGraphGroupEdit(py::object makeEditsObj);

        /**
         * @brief
         @rst
         Calls #makeEditsObj for each group of 'graphGroups', in order, and calls 'method' on every
         edit it returns -- ``"edit"`` with ``(graphGroups, modType, modName)``, or ``"editFromIni"``
         with 'ini' before 'modType'
         @endrst
         *
         * @param graphGroups The Python list of groups
         * @param ini The .ini file, or ``None`` for ``"edit"``
         * @param modType The type of mod to fix
         * @param modName The name of the mod to fix to
         * @param fromIni Whether to call ``editFromIni`` rather than ``edit``
         *
         * @return 'graphGroups'
         */
        py::list run(py::list graphGroups, const py::object &ini, const py::object &modType, const std::string &modName,
                     bool fromIni);
};


void initCppIterGraphGroupEdit(pybind11::module_ &m);

#endif
