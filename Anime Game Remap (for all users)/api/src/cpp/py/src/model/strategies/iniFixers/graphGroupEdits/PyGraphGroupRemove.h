#ifndef PY_GRAPH_GROUP_REMOVE_H
#define PY_GRAPH_GROUP_REMOVE_H

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
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupRemove.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphGroupRemove`\\<std::string, std::string\\>
 :raw-html:`<br />` :raw-html:`<br />`

 Holds the exact `Python`_ object given for ``iniIndices``, as `PyGraphRemove` holds its
 ``graphIds``: ``someEdit.iniIndices is theListYouPassed`` holds, and mutating that list afterwards
 changes what the edit does. The C++ member is re-derived from it at the start of every ``edit``
 (see #refresh)
 @endrst
 */
class PyGraphGroupRemove: public AGRC::GraphGroupRemove<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphGroupRemove<std::string, std::string>;

        /**
         * @brief The exact `Python`_ object given for ``iniIndices`` -- a list of ``int``, or ``None`` for every group
         */
        py::object iniIndicesObj;

        /**
         * @brief Constructs a new group-removing edit
         *
         * @param iniIndicesObj The Python list of ``.ini`` indices, or ``None`` for every group
         */
        explicit PyGraphGroupRemove(py::object iniIndicesObj);

        /**
         * @brief Re-derives the core's \ref iniIndices from #iniIndicesObj
         */
        void refresh();
};


/**
 * @brief Registers the Python-facing ``GraphGroupRemove``
 *
 * @param m The module to register into
 */
void initCppGraphGroupRemove(pybind11::module_ &m);

#endif
