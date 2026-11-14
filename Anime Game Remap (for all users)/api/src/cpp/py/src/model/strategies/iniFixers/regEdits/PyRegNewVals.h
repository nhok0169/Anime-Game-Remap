#ifndef AGRemapPyBind_PyRegNewVals_H
#define AGRemapPyBind_PyRegNewVals_H

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
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 Marks a callable in a `PyRegNewVals` value slot as wanting the value ALREADY THERE
 :raw-html:`<br />` :raw-html:`<br />`

 A bare callable in that slot already means something -- ``newVal(modType)``, `AGRC::RegNewVals`'s
 ``ValProducer`` -- so the one that also reads the old value has to say which it is, the way
 `PyReplaceList` and `PyReplaceIf` say which of ``replaceVals``' forms they are. Wrapped in this, the
 callable is invoked as ``newVal(oldValue, modType)``
 @endrst
 */
class PyFromOldVal {
    public:
        explicit PyFromOldVal(py::object producer): producer_(std::move(producer)) {
            // The same explicit check PyReplaceIf makes of its predicate, for the same reason: a
            // non-callable here would otherwise fail deep inside `edit`, one mod into a run.
            if (!PyCallable_Check(producer_.ptr())) {
                throw py::type_error("FromOldVal(): 'producer' must be callable");
            }
        }

        /**
         * @brief Retrieves the raw Python callable, for a caller that supplies its own argument list
         */
        const py::object& producer() const { return producer_; }

    private:
        py::object producer_;
};


/**
 * @brief
 @rst
 The `Python`_ ``RegNewVals`` is :cpp:class:`AGRemapCore::RegNewVals` itself: its arguments are parsed into the C++ members
 when it is built or they are set, and read back from them -- so one built in `Python`_ runs in a C++ fixer's list, and
 one built in C++ reads in `Python`_ as a ``RegNewVals``
 @endrst
 */
using PyRegNewVals = AGRC::RegNewVals<std::string, std::string>;


void initCppRegNewVals(pybind11::module_ &m);

#endif
