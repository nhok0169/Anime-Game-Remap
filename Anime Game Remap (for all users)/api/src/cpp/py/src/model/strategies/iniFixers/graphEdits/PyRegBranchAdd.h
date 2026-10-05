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

 Holds the exact `Python`_ callable given for ``branchOf`` -- a ``std::function`` cannot be handed back
 to `Python`_ as the callable it was built from -- and re-wraps it as the core's \ref branchOf at the
 start of every ``edit`` (see #refresh), so assigning a new callable takes effect on the next edit
 @endrst
 */
class PyRegBranchAdd: public AGRC::RegBranchAdd<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegBranchAdd<std::string, std::string>;

        /**
         * @brief The exact `Python`_ callable given for ``branchOf``, or ``None``
         */
        py::object branchOfObj;

        /**
         * @brief Constructs a new branch-adding edit
         *
         * @param branchOfObj The Python callable deciding what belongs in each branch, or ``None``
         */
        explicit PyRegBranchAdd(py::object branchOfObj);

        /**
         * @brief Re-wraps #branchOfObj as the core's \ref branchOf
         */
        void refresh();
};


/**
 * @brief Registers the Python-facing ``RegBranchAdd``
 *
 * @param m The module to register into
 */
void initCppRegBranchAdd(pybind11::module_ &m);

#endif
