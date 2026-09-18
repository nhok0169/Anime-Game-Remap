#ifndef PyRegBottomAdd_H
#define PyRegBottomAdd_H

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
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBottomAdd.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegBottomAdd`\\<std::string, std::string\\>
 :raw-html:`<br />` :raw-html:`<br />`

 Simpler than its neighbours: this edit holds no predicates and no register map, so there is no
 raw `Python`_ object to keep alongside the core's own state and no derived state to re-sync. The
 only field is the `KVP`_ list, which round-trips through the same ``parseAdditions`` /
 ``additionsToPy`` pair `PyRegSurroundedAdd` and `PyRegDelimitedAdd` use
 @endrst
 */
class PyRegBottomAdd: public AGRC::RegBottomAdd<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegBottomAdd<std::string, std::string>;

        /**
         * @brief Constructs a new bottom-adding edit
         *
         * @param additionsObj The `KVP`_ tuple(s) to add -- one ``(key, value)`` tuple, or a list of them
         */
        explicit PyRegBottomAdd(py::object additionsObj);
};


/**
 * @brief Registers the Python-facing ``RegBottomAdd``
 *
 * @param m The module to register into
 */
void initCppRegBottomAdd(pybind11::module_ &m);

#endif
