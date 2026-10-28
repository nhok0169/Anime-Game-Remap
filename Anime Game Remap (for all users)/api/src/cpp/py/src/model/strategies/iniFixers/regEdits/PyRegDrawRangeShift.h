#ifndef PY_REG_DRAW_RANGE_SHIFT_H
#define PY_REG_DRAW_RANGE_SHIFT_H

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

#include "PyBaseRegEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegDrawRangeShift.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegDrawRangeShift`\\<std::string, std::string\\>, with the
 ``refresh`` every bound reg edit's ``edit`` calls (a no-op: this edit holds no `Python`_ object)
 @endrst
 */
class PyRegDrawRangeShift: public AGRC::RegDrawRangeShift<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegDrawRangeShift<std::string, std::string>;

        using Core::Core;

        /**
         * @copydoc PyRegAdd::refresh
         */
        void refresh(const py::object &modType);
};


/**
 * @brief Registers the Python-facing ``RegDrawRangeShift``
 *
 * @param m The module to register into
 */
void initCppRegDrawRangeShift(pybind11::module_ &m);

#endif
