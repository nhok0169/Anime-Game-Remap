#ifndef PY_REG_BRANCH_ADD_H
#define PY_REG_BRANCH_ADD_H

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

#include "PyBaseIniGraphEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBranchAdd.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegBranchAdd`\\<std::string, std::string\\>
 :raw-html:`<br />` :raw-html:`<br />`

 The ``branchOf`` given in `Python`_ is the core's \ref branchOf when the edit is built or the property is set -- so one
 built in `Python`_ runs in a C++ fixer's list -- and reads back as the callable it was given
 @endrst
 */
class PyRegBranchAdd: public AGRC::RegBranchAdd<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegBranchAdd<std::string, std::string>;

        /**
         * @brief Constructs a new branch-adding edit
         *
         * @param branchOfObj The Python callable deciding what belongs in each branch, or ``None``
         */
        explicit PyRegBranchAdd(py::object branchOfObj);

        /**
         * @brief Makes a Python callable (or ``None``) the core's \ref branchOf
         */
        void setBranchOf(const py::object &branchOfObj);

        /**
         * @brief The core's \ref branchOf as Python: the callable it was given, else the C++ function wrapped, else ``None``
         */
        py::object branchOfToPy() const;
};


/**
 * @brief Registers the Python-facing ``RegBranchAdd``
 *
 * @param m The module to register into
 */
void initCppRegBranchAdd(pybind11::module_ &m);

#endif
